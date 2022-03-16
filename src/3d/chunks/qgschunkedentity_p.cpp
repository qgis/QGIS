/***************************************************************************
  qgschunkedentity_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgschunkedentity_p.h"

#include <QElapsedTimer>
#include <QVector4D>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickTriangleEvent>
#include <Qt3DRender/QBuffer>

#include "qgs3dutils.h"
#include "qgschunkboundsentity_p.h"
#include "qgschunklist_p.h"
#include "qgschunkloader_p.h"
#include "qgschunknode_p.h"
#include "qgstessellatedpolygongeometry.h"

#include "qgseventtracing.h"

#include <queue>

///@cond PRIVATE

static float screenSpaceError( float epsilon, float distance, float screenSize, float fov )
{
  /* This routine approximately calculates how an error (epsilon) of an object in world coordinates
   * at given distance (between camera and the object) will look like in screen coordinates.
   *
   * the math below simply uses triangle similarity:
   *
   *             epsilon                       phi
   *   -----------------------------  = ----------------
   *   [ frustum width at distance ]    [ screen width ]
   *
   * Then we solve for phi, substituting [frustum width at distance] = 2 * distance * tan(fov / 2)
   *
   *  ________xxx__      xxx = real world error (epsilon)
   *  \     |     /        x = screen space error (phi)
   *   \    |    /
   *    \___|_x_/   near plane (screen space)
   *     \  |  /
   *      \ | /
   *       \|/    angle = field of view
   *       camera
   */
  float phi = epsilon * screenSize / ( 2 * distance * tan( fov * M_PI / ( 2 * 180 ) ) );
  return phi;
}

static float screenSpaceError( QgsChunkNode *node, const QgsChunkedEntity::SceneState &state )
{
  if ( node->error() <= 0 ) //it happens for meshes
    return 0;

  float dist = node->bbox().distanceFromPoint( state.cameraPos );

  // TODO: what to do when distance == 0 ?

  float sse = screenSpaceError( node->error(), dist, state.screenSizePx, state.cameraFov );
  return sse;
}

QgsChunkedEntity::QgsChunkedEntity( float tau, QgsChunkLoaderFactory *loaderFactory, bool ownsFactory, int primitiveBudget, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mTau( tau )
  , mChunkLoaderFactory( loaderFactory )
  , mOwnsFactory( ownsFactory )
  , mPrimitivesBudget( primitiveBudget )
{
  mRootNode = loaderFactory->createRootNode();
  mChunkLoaderQueue = new QgsChunkList;
  mReplacementQueue = new QgsChunkList;
}


QgsChunkedEntity::~QgsChunkedEntity()
{
  // derived classes have to make sure that any pending active job has finished / been canceled
  // before getting to this destructor - here it would be too late to cancel them
  // (e.g. objects required for loading/updating have been deleted already)
  Q_ASSERT( mActiveJobs.isEmpty() );

  // clean up any pending load requests
  while ( !mChunkLoaderQueue->isEmpty() )
  {
    QgsChunkListEntry *entry = mChunkLoaderQueue->takeFirst();
    QgsChunkNode *node = entry->chunk;

    if ( node->state() == QgsChunkNode::QueuedForLoad )
      node->cancelQueuedForLoad();
    else if ( node->state() == QgsChunkNode::QueuedForUpdate )
      node->cancelQueuedForUpdate();
    else
      Q_ASSERT( false );  // impossible!
  }

  delete mChunkLoaderQueue;

  while ( !mReplacementQueue->isEmpty() )
  {
    QgsChunkListEntry *entry = mReplacementQueue->takeFirst();

    // remove loaded data from node
    entry->chunk->unloadChunk(); // also deletes the entry
  }

  delete mReplacementQueue;
  delete mRootNode;

  if ( mOwnsFactory )
  {
    delete mChunkLoaderFactory;
  }
}


void QgsChunkedEntity::update( const SceneState &state )
{
  if ( !mIsValid )
    return;

  QElapsedTimer t;
  t.start();

  int oldJobsCount = pendingJobsCount();

  QSet<QgsChunkNode *> activeBefore = qgis::listToSet( mActiveNodes );
  mActiveNodes.clear();
  mFrustumCulled = 0;
  mCurrentTime = QTime::currentTime();

  update( mRootNode, state );

  int enabled = 0, disabled = 0, unloaded = 0;

  for ( QgsChunkNode *node : std::as_const( mActiveNodes ) )
  {
    if ( activeBefore.contains( node ) )
    {
      activeBefore.remove( node );
    }
    else
    {
      if ( !node->entity() )
      {
        QgsDebugMsg( "Active node has null entity - this should never happen!" );
        continue;
      }
      node->entity()->setEnabled( true );
      ++enabled;
    }
  }

  // disable those that were active but will not be anymore
  for ( QgsChunkNode *node : activeBefore )
  {
    if ( !node->entity() )
    {
      QgsDebugMsg( "Active node has null entity - this should never happen!" );
      continue;
    }
    node->entity()->setEnabled( false );
    ++disabled;
  }

  double usedGpuMemory = QgsChunkedEntity::calculateEntityGpuMemorySize( this );

  // unload those that are over the limit for replacement
  // TODO: what to do when our cache is too small and nodes are being constantly evicted + loaded again
  while ( usedGpuMemory > mGpuMemoryLimit )
  {
    QgsChunkListEntry *entry = mReplacementQueue->takeLast();
    usedGpuMemory -= QgsChunkedEntity::calculateEntityGpuMemorySize( entry->chunk->entity() );
    entry->chunk->unloadChunk();  // also deletes the entry
    mActiveNodes.removeOne( entry->chunk );
    ++unloaded;
  }

  if ( mBboxesEntity )
  {
    QList<QgsAABB> bboxes;
    for ( QgsChunkNode *n : std::as_const( mActiveNodes ) )
      bboxes << n->bbox();
    mBboxesEntity->setBoxes( bboxes );
  }

  // start a job from queue if there is anything waiting
  startJobs();

  mNeedsUpdate = false;  // just updated

  if ( pendingJobsCount() != oldJobsCount )
    emit pendingJobsCountChanged();

  QgsDebugMsgLevel( QStringLiteral( "update: active %1 enabled %2 disabled %3 | culled %4 | loading %5 loaded %6 | unloaded %7 elapsed %8ms" ).arg( mActiveNodes.count() )
                    .arg( enabled )
                    .arg( disabled )
                    .arg( mFrustumCulled )
                    .arg( mReplacementQueue->count() )
                    .arg( unloaded )
                    .arg( t.elapsed() ), 2 );
}

void QgsChunkedEntity::setShowBoundingBoxes( bool enabled )
{
  if ( ( enabled && mBboxesEntity ) || ( !enabled && !mBboxesEntity ) )
    return;

  if ( enabled )
  {
    mBboxesEntity = new QgsChunkBoundsEntity( this );
  }
  else
  {
    mBboxesEntity->deleteLater();
    mBboxesEntity = nullptr;
  }
}

void QgsChunkedEntity::updateNodes( const QList<QgsChunkNode *> &nodes, QgsChunkQueueJobFactory *updateJobFactory )
{
  for ( QgsChunkNode *node : nodes )
  {
    if ( node->state() == QgsChunkNode::QueuedForUpdate )
    {
      mChunkLoaderQueue->takeEntry( node->loaderQueueEntry() );
      node->cancelQueuedForUpdate();
    }
    else if ( node->state() == QgsChunkNode::Updating )
    {
      cancelActiveJob( node->updater() );
    }

    Q_ASSERT( node->state() == QgsChunkNode::Loaded );

    QgsChunkListEntry *entry = new QgsChunkListEntry( node );
    node->setQueuedForUpdate( entry, updateJobFactory );
    mChunkLoaderQueue->insertLast( entry );
  }

  // trigger update
  startJobs();
}

int QgsChunkedEntity::pendingJobsCount() const
{
  return mChunkLoaderQueue->count() + mActiveJobs.count();
}

struct ResidencyRequest
{
  QgsChunkNode *node = nullptr;
  float dist = 0.0;
  int level = -1;
  ResidencyRequest() = default;
  ResidencyRequest(
    QgsChunkNode *n,
    float d,
    int l )
    : node( n )
    , dist( d )
    , level( l )
  {}
};

struct
{
  bool operator()( const ResidencyRequest &request, const ResidencyRequest &otherRequest ) const
  {
    if ( request.level == otherRequest.level )
      return request.dist > otherRequest.dist;
    return request.level > otherRequest.level;
  }
} ResidencyRequestSorter;

void QgsChunkedEntity::update( QgsChunkNode *root, const SceneState &state )
{
  QSet<QgsChunkNode *> nodes;
  QVector<ResidencyRequest> residencyRequests;

  using slotItem = std::pair<QgsChunkNode *, float>;
  auto cmp_funct = []( slotItem & p1, slotItem & p2 )
  {
    return p1.second <= p2.second;
  };
  int renderedCount = 0;
  std::priority_queue<slotItem, std::vector<slotItem>, decltype( cmp_funct )> pq( cmp_funct );
  pq.push( std::make_pair( root, screenSpaceError( root, state ) ) );
  while ( !pq.empty() && renderedCount <= mPrimitivesBudget )
  {
    slotItem s = pq.top();
    pq.pop();
    QgsChunkNode *node = s.first;

    if ( Qgs3DUtils::isCullable( node->bbox(), state.viewProjectionMatrix ) )
    {
      ++mFrustumCulled;
      continue;
    }

    // ensure we have child nodes (at least skeletons) available, if any
    if ( node->childCount() == -1 )
      node->populateChildren( mChunkLoaderFactory->createChildren( node ) );

    // make sure all nodes leading to children are always loaded
    // so that zooming out does not create issues
    double dist = node->bbox().center().distanceToPoint( state.cameraPos );
    residencyRequests.push_back( ResidencyRequest( node, dist, node->level() ) );

    if ( !node->entity() )
    {
      // this happens initially when root node is not ready yet
      continue;
    }
    bool becomesActive = false;

    // QgsDebugMsgLevel( QStringLiteral( "%1|%2|%3  %4  %5" ).arg( node->tileId().x ).arg( node->tileId().y ).arg( node->tileId().z ).arg( mTau ).arg( screenSpaceError( node, state ) ), 2 );
    if ( node->childCount() == 0 )
    {
      // there's no children available for this node, so regardless of whether it has an acceptable error
      // or not, it's the best we'll ever get...
      becomesActive = true;
    }
    else if ( mTau > 0 && screenSpaceError( node, state ) <= mTau )
    {
      // acceptable error for the current chunk - let's render it
      becomesActive = true;
    }
    else if ( node->allChildChunksResident( mCurrentTime ) )
    {
      // error is not acceptable and children are ready to be used - recursive descent
      if ( mAdditiveStrategy )
      {
        // With additive strategy enabled, also all parent nodes are added to active nodes.
        // This is desired when child nodes add more detailed data rather than just replace
        // coarser data in parents. We use this e.g. with point cloud data.
        becomesActive = true;
      }
      QgsChunkNode *const *children = node->children();
      for ( int i = 0; i < node->childCount(); ++i )
        pq.push( std::make_pair( children[i], screenSpaceError( children[i], state ) ) );
    }
    else
    {
      // error is not acceptable but children are not ready either - still use parent but request children
      becomesActive = true;

      QgsChunkNode *const *children = node->children();
      for ( int i = 0; i < node->childCount(); ++i )
      {
        double dist = children[i]->bbox().center().distanceToPoint( state.cameraPos );
        residencyRequests.push_back( ResidencyRequest( children[i], dist, children[i]->level() ) );
      }
    }
    if ( becomesActive )
    {
      mActiveNodes << node;
      // if we are not using additive strategy we need to make sure the parent primitives are not counted
      if ( !mAdditiveStrategy && node->parent() && nodes.contains( node->parent() ) )
      {
        nodes.remove( node->parent() );
        renderedCount -= mChunkLoaderFactory->primitivesCount( node->parent() );
      }
      renderedCount += mChunkLoaderFactory->primitivesCount( node );
      nodes.insert( node );
    }
  }

  // sort nodes by their level and their distance from the camera
  std::sort( residencyRequests.begin(), residencyRequests.end(), ResidencyRequestSorter );
  for ( const auto &request : residencyRequests )
    requestResidency( request.node );
}

void QgsChunkedEntity::requestResidency( QgsChunkNode *node )
{
  if ( node->state() == QgsChunkNode::Loaded || node->state() == QgsChunkNode::QueuedForUpdate || node->state() == QgsChunkNode::Updating )
  {
    Q_ASSERT( node->replacementQueueEntry() );
    Q_ASSERT( node->entity() );
    mReplacementQueue->takeEntry( node->replacementQueueEntry() );
    mReplacementQueue->insertFirst( node->replacementQueueEntry() );
  }
  else if ( node->state() == QgsChunkNode::QueuedForLoad )
  {
    // move to the front of loading queue
    Q_ASSERT( node->loaderQueueEntry() );
    Q_ASSERT( !node->loader() );
    if ( node->loaderQueueEntry()->prev || node->loaderQueueEntry()->next )
    {
      mChunkLoaderQueue->takeEntry( node->loaderQueueEntry() );
      mChunkLoaderQueue->insertFirst( node->loaderQueueEntry() );
    }
  }
  else if ( node->state() == QgsChunkNode::Loading )
  {
    // the entry is being currently processed - nothing to do really
  }
  else if ( node->state() == QgsChunkNode::Skeleton )
  {
    if ( !node->hasData() )
      return;   // no need to load (we already tried but got nothing back)

    // add to the loading queue
    QgsChunkListEntry *entry = new QgsChunkListEntry( node );
    node->setQueuedForLoad( entry );
    mChunkLoaderQueue->insertFirst( entry );
  }
  else
    Q_ASSERT( false && "impossible!" );
}


void QgsChunkedEntity::onActiveJobFinished()
{
  int oldJobsCount = pendingJobsCount();

  QgsChunkQueueJob *job = qobject_cast<QgsChunkQueueJob *>( sender() );
  Q_ASSERT( job );
  Q_ASSERT( mActiveJobs.contains( job ) );

  QgsChunkNode *node = job->chunk();

  if ( QgsChunkLoader *loader = qobject_cast<QgsChunkLoader *>( job ) )
  {
    Q_ASSERT( node->state() == QgsChunkNode::Loading );
    Q_ASSERT( node->loader() == loader );

    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Load " ) + node->tileId().text(), node->tileId().text() );
    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Load" ), node->tileId().text() );

    QgsEventTracing::ScopedEvent e( "3D", QString( "create" ) );
    // mark as loaded + create entity
    Qt3DCore::QEntity *entity = node->loader()->createEntity( this );

    if ( entity )
    {
      // load into node (should be in main thread again)
      node->setLoaded( entity );

      mReplacementQueue->insertFirst( node->replacementQueueEntry() );

      if ( mPickingEnabled )
      {
        Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker( node->entity() );
        node->entity()->addComponent( picker );
        connect( picker, &Qt3DRender::QObjectPicker::clicked, this, &QgsChunkedEntity::onPickEvent );
      }

      emit newEntityCreated( entity );
    }
    else
    {
      node->setHasData( false );
      node->cancelLoading();
    }

    // now we need an update!
    mNeedsUpdate = true;
  }
  else
  {
    Q_ASSERT( node->state() == QgsChunkNode::Updating );
    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Update" ), node->tileId().text() );
    node->setUpdated();
  }

  // cleanup the job that has just finished
  mActiveJobs.removeOne( job );
  job->deleteLater();

  // start another job - if any
  startJobs();

  if ( pendingJobsCount() != oldJobsCount )
    emit pendingJobsCountChanged();
}

void QgsChunkedEntity::startJobs()
{
  while ( mActiveJobs.count() < 4 )
  {
    if ( mChunkLoaderQueue->isEmpty() )
      return;

    QgsChunkListEntry *entry = mChunkLoaderQueue->takeFirst();
    Q_ASSERT( entry );
    QgsChunkNode *node = entry->chunk;
    delete entry;

    QgsChunkQueueJob *job = startJob( node );
    mActiveJobs.append( job );
  }
}

QgsChunkQueueJob *QgsChunkedEntity::startJob( QgsChunkNode *node )
{
  if ( node->state() == QgsChunkNode::QueuedForLoad )
  {
    QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "Load" ), node->tileId().text() );
    QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "Load " ) + node->tileId().text(), node->tileId().text() );

    QgsChunkLoader *loader = mChunkLoaderFactory->createChunkLoader( node );
    connect( loader, &QgsChunkQueueJob::finished, this, &QgsChunkedEntity::onActiveJobFinished );
    node->setLoading( loader );
    return loader;
  }
  else if ( node->state() == QgsChunkNode::QueuedForUpdate )
  {
    QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "Update" ), node->tileId().text() );

    node->setUpdating();
    connect( node->updater(), &QgsChunkQueueJob::finished, this, &QgsChunkedEntity::onActiveJobFinished );
    return node->updater();
  }
  else
  {
    Q_ASSERT( false );  // not possible
    return nullptr;
  }
}

void QgsChunkedEntity::cancelActiveJob( QgsChunkQueueJob *job )
{
  Q_ASSERT( job );

  QgsChunkNode *node = job->chunk();

  if ( qobject_cast<QgsChunkLoader *>( job ) )
  {
    // return node back to skeleton
    node->cancelLoading();

    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Load " ) + node->tileId().text(), node->tileId().text() );
    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Load" ), node->tileId().text() );
  }
  else
  {
    // return node back to loaded state
    node->cancelUpdating();

    QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Update" ), node->tileId().text() );
  }

  job->cancel();
  mActiveJobs.removeOne( job );
  job->deleteLater();
}

void QgsChunkedEntity::cancelActiveJobs()
{
  while ( !mActiveJobs.isEmpty() )
  {
    cancelActiveJob( mActiveJobs.takeFirst() );
  }
}


void QgsChunkedEntity::setPickingEnabled( bool enabled )
{
  if ( mPickingEnabled == enabled )
    return;

  mPickingEnabled = enabled;

  if ( enabled )
  {
    QgsChunkListEntry *entry = mReplacementQueue->first();
    while ( entry )
    {
      QgsChunkNode *node = entry->chunk;
      Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker( node->entity() );
      node->entity()->addComponent( picker );
      connect( picker, &Qt3DRender::QObjectPicker::clicked, this, &QgsChunkedEntity::onPickEvent );

      entry = entry->next;
    }
  }
  else
  {
    for ( Qt3DRender::QObjectPicker *picker : findChildren<Qt3DRender::QObjectPicker *>() )
      picker->deleteLater();
  }
}

void QgsChunkedEntity::onPickEvent( Qt3DRender::QPickEvent *event )
{
  Qt3DRender::QPickTriangleEvent *triangleEvent = qobject_cast<Qt3DRender::QPickTriangleEvent *>( event );
  if ( !triangleEvent )
    return;

  Qt3DRender::QObjectPicker *picker = qobject_cast<Qt3DRender::QObjectPicker *>( sender() );
  if ( !picker )
    return;

  Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( picker->parent() );
  if ( !entity )
    return;

  // go figure out feature ID from the triangle index
  QgsFeatureId fid = FID_NULL;
  for ( Qt3DRender::QGeometryRenderer *geomRenderer : entity->findChildren<Qt3DRender::QGeometryRenderer *>() )
  {
    // unfortunately we can't access which sub-entity triggered the pick event
    // so as a temporary workaround let's just ignore the entity with selection
    // and hope the event was the main entity (QTBUG-58206)
    if ( geomRenderer->objectName() != QLatin1String( "main" ) )
      continue;

    if ( QgsTessellatedPolygonGeometry *g = qobject_cast<QgsTessellatedPolygonGeometry *>( geomRenderer->geometry() ) )
    {
      fid = g->triangleIndexToFeatureId( triangleEvent->triangleIndex() );
      if ( !FID_IS_NULL( fid ) )
        break;
    }
  }

  if ( !FID_IS_NULL( fid ) )
  {
    emit pickedObject( event, fid );
  }
}

double QgsChunkedEntity::calculateEntityGpuMemorySize( Qt3DCore::QEntity *entity )
{
  long long usedGpuMemory = 0;
  for ( Qt3DRender::QBuffer *buffer : entity->findChildren<Qt3DRender::QBuffer *>() )
  {
    usedGpuMemory += buffer->data().size();
  }
  for ( Qt3DRender::QTexture2D *tex : entity->findChildren<Qt3DRender::QTexture2D *>() )
  {
    // TODO : lift the assumption that the texture is RGBA
    usedGpuMemory += tex->width() * tex->height() * 4;
  }
  return usedGpuMemory / 1024.0 / 1024.0;
}

/// @endcond
