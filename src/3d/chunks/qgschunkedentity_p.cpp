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

#include "qgs3dutils.h"
#include "qgschunkboundsentity_p.h"
#include "qgschunklist_p.h"
#include "qgschunkloader_p.h"
#include "qgschunknode_p.h"

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
  float dist = node->bbox().distanceFromPoint( state.cameraPos );

  // TODO: what to do when distance == 0 ?

  float sse = screenSpaceError( node->error(), dist, state.screenSizePx, state.cameraFov );
  return sse;
}

QgsChunkedEntity::QgsChunkedEntity( const QgsAABB &rootBbox, float rootError, float tau, int maxLevel, QgsChunkLoaderFactory *loaderFactory, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mTau( tau )
  , mMaxLevel( maxLevel )
  , mChunkLoaderFactory( loaderFactory )
{
  mRootNode = new QgsChunkNode( 0, 0, 0, rootBbox, rootError );
  mChunkLoaderQueue = new QgsChunkList;
  mReplacementQueue = new QgsChunkList;
}


QgsChunkedEntity::~QgsChunkedEntity()
{
  // derived classes have to make sure that any pending active job has finished / been canceled
  // before getting to this destructor - here it would be too late to cancel them
  // (e.g. objects required for loading/updating have been deleted already)
  Q_ASSERT( !mActiveJob );

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

  // TODO: shall we own the factory or not?
  //delete chunkLoaderFactory;
}


void QgsChunkedEntity::update( const SceneState &state )
{
  QElapsedTimer t;
  t.start();

  int oldJobsCount = pendingJobsCount();

  QSet<QgsChunkNode *> activeBefore = QSet<QgsChunkNode *>::fromList( mActiveNodes );
  mActiveNodes.clear();
  mFrustumCulled = 0;
  mCurrentTime = QTime::currentTime();

  update( mRootNode, state );

  int enabled = 0, disabled = 0, unloaded = 0;

  Q_FOREACH ( QgsChunkNode *node, mActiveNodes )
  {
    if ( activeBefore.contains( node ) )
      activeBefore.remove( node );
    else
    {
      node->entity()->setEnabled( true );
      ++enabled;
    }
  }

  // disable those that were active but will not be anymore
  Q_FOREACH ( QgsChunkNode *node, activeBefore )
  {
    node->entity()->setEnabled( false );
    ++disabled;
  }

  // unload those that are over the limit for replacement
  // TODO: what to do when our cache is too small and nodes are being constantly evicted + loaded again
  while ( mReplacementQueue->count() > mMaxLoadedChunks )
  {
    QgsChunkListEntry *entry = mReplacementQueue->takeLast();
    entry->chunk->unloadChunk();  // also deletes the entry
    ++unloaded;
  }

  if ( mBboxesEntity )
  {
    QList<QgsAABB> bboxes;
    Q_FOREACH ( QgsChunkNode *n, mActiveNodes )
      bboxes << n->bbox();
    mBboxesEntity->setBoxes( bboxes );
  }

  // start a job from queue if there is anything waiting
  if ( !mActiveJob )
    startJob();

  mNeedsUpdate = false;  // just updated

  if ( pendingJobsCount() != oldJobsCount )
    emit pendingJobsCountChanged();

//  qDebug() << "update: active " << mActiveNodes.count() << " enabled " << enabled << " disabled " << disabled << " | culled " << mFrustumCulled << " | loading " << mChunkLoaderQueue->count() << " loaded " << mReplacementQueue->count() << " | unloaded " << unloaded << " elapsed " << t.elapsed() << "ms";
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
  Q_FOREACH ( QgsChunkNode *node, nodes )
  {
    if ( node->state() == QgsChunkNode::QueuedForUpdate )
    {
      mChunkLoaderQueue->takeEntry( node->loaderQueueEntry() );
      node->cancelQueuedForUpdate();
    }
    else if ( node->state() == QgsChunkNode::Updating )
    {
      cancelActiveJob();  // we have currently just one active job so that must be it
    }

    Q_ASSERT( node->state() == QgsChunkNode::Loaded );

    QgsChunkListEntry *entry = new QgsChunkListEntry( node );
    node->setQueuedForUpdate( entry, updateJobFactory );
    mChunkLoaderQueue->insertLast( entry );
  }

  // trigger update
  if ( !mActiveJob )
    startJob();
}

int QgsChunkedEntity::pendingJobsCount() const
{
  return mChunkLoaderQueue->count() + ( mActiveJob ? 1 : 0 );
}


void QgsChunkedEntity::update( QgsChunkNode *node, const SceneState &state )
{
  if ( Qgs3DUtils::isCullable( node->bbox(), state.viewProjectionMatrix ) )
  {
    ++mFrustumCulled;
    return;
  }

  node->ensureAllChildrenExist();

  // make sure all nodes leading to children are always loaded
  // so that zooming out does not create issues
  requestResidency( node );

  if ( !node->entity() )
  {
    // this happens initially when root node is not ready yet
    return;
  }

  //qDebug() << node->x << "|" << node->y << "|" << node->z << "  " << tau << "  " << screenSpaceError(node, state);

  if ( screenSpaceError( node, state ) <= mTau )
  {
    // acceptable error for the current chunk - let's render it

    mActiveNodes << node;
  }
  else if ( node->allChildChunksResident( mCurrentTime ) )
  {
    // error is not acceptable and children are ready to be used - recursive descent

    QgsChunkNode *const *children = node->children();
    for ( int i = 0; i < 4; ++i )
      update( children[i], state );
  }
  else
  {
    // error is not acceptable but children are not ready either - still use parent but request children

    mActiveNodes << node;

    if ( node->level() < mMaxLevel )
    {
      QgsChunkNode *const *children = node->children();
      for ( int i = 0; i < 4; ++i )
        requestResidency( children[i] );
    }
  }
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
  Q_ASSERT( job == mActiveJob );

  QgsChunkNode *node = job->chunk();

  if ( QgsChunkLoader *loader = qobject_cast<QgsChunkLoader *>( job ) )
  {
    Q_ASSERT( node->state() == QgsChunkNode::Loading );
    Q_ASSERT( node->loader() == loader );

    // mark as loaded + create entity
    Qt3DCore::QEntity *entity = node->loader()->createEntity( this );

    if ( entity )
    {
      // load into node (should be in main thread again)
      node->setLoaded( entity );

      mReplacementQueue->insertFirst( node->replacementQueueEntry() );
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
    node->setUpdated();
  }

  // cleanup the job that has just finished
  mActiveJob->deleteLater();
  mActiveJob = nullptr;

  // start another job - if any
  startJob();

  if ( pendingJobsCount() != oldJobsCount )
    emit pendingJobsCountChanged();
}

void QgsChunkedEntity::startJob()
{
  Q_ASSERT( !mActiveJob );
  if ( mChunkLoaderQueue->isEmpty() )
    return;

  QgsChunkListEntry *entry = mChunkLoaderQueue->takeFirst();
  Q_ASSERT( entry );
  QgsChunkNode *node = entry->chunk;
  delete entry;

  if ( node->state() == QgsChunkNode::QueuedForLoad )
  {
    QgsChunkLoader *loader = mChunkLoaderFactory->createChunkLoader( node );
    connect( loader, &QgsChunkQueueJob::finished, this, &QgsChunkedEntity::onActiveJobFinished );
    node->setLoading( loader );
    mActiveJob = loader;
  }
  else if ( node->state() == QgsChunkNode::QueuedForUpdate )
  {
    node->setUpdating();
    connect( node->updater(), &QgsChunkQueueJob::finished, this, &QgsChunkedEntity::onActiveJobFinished );
    mActiveJob = node->updater();
  }
  else
    Q_ASSERT( false );  // not possible
}

void QgsChunkedEntity::cancelActiveJob()
{
  Q_ASSERT( mActiveJob );

  QgsChunkNode *node = mActiveJob->chunk();

  if ( qobject_cast<QgsChunkLoader *>( mActiveJob ) )
  {
    // return node back to skeleton
    node->cancelLoading();
  }
  else
  {
    // return node back to loaded state
    node->cancelUpdating();
  }

  mActiveJob->cancel();
  mActiveJob->deleteLater();
  mActiveJob = nullptr;
}

/// @endcond
