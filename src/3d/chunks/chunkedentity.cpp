#include <QVector4D>
#include "chunkedentity.h"
#include "chunkboundsentity.h"
#include "chunklist.h"
#include "chunkloader.h"
#include "chunknode.h"
#include "qgs3dutils.h"

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

static float screenSpaceError( ChunkNode *node, const SceneState &state )
{
  float dist = node->bbox.distanceFromPoint( state.cameraPos );

  // TODO: what to do when distance == 0 ?

  float sse = screenSpaceError( node->error, dist, state.screenSizePx, state.cameraFov );
  return sse;
}

ChunkedEntity::ChunkedEntity( const AABB &rootBbox, float rootError, float tau, int maxLevel, ChunkLoaderFactory *loaderFactory, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , needsUpdate( false )
  , tau( tau )
  , maxLevel( maxLevel )
  , chunkLoaderFactory( loaderFactory )
  , maxLoadedChunks( 512 )
  , bboxesEntity( nullptr )
{
  rootNode = new ChunkNode( 0, 0, 0, rootBbox, rootError );
  chunkLoaderQueue = new ChunkList;
  replacementQueue = new ChunkList;
}


ChunkedEntity::~ChunkedEntity()
{
  // derived classes have to make sure that any pending active job has finished / been cancelled
  // before getting to this destructor - here it would be too late to cancel them
  // (e.g. objects required for loading/updating have been deleted already)
  Q_ASSERT( !activeJob );

  // clean up any pending load requests
  while ( !chunkLoaderQueue->isEmpty() )
  {
    ChunkListEntry *entry = chunkLoaderQueue->takeFirst();
    ChunkNode *node = entry->chunk;

    if ( node->state == ChunkNode::QueuedForLoad )
      node->cancelQueuedForLoad();
    else if ( node->state == ChunkNode::QueuedForUpdate )
      node->cancelQueuedForUpdate();
    else
      Q_ASSERT( false );  // impossible!
  }

  delete chunkLoaderQueue;

  while ( !replacementQueue->isEmpty() )
  {
    ChunkListEntry *entry = replacementQueue->takeFirst();

    // remove loaded data from node
    entry->chunk->unloadChunk(); // also deletes the entry
  }

  delete replacementQueue;
  delete rootNode;

  // TODO: shall we own the factory or not?
  //delete chunkLoaderFactory;
}

#include <QElapsedTimer>

void ChunkedEntity::update( const SceneState &state )
{
  QElapsedTimer t;
  t.start();

  QSet<ChunkNode *> activeBefore = QSet<ChunkNode *>::fromList( activeNodes );
  activeNodes.clear();
  frustumCulled = 0;
  currentTime = QTime::currentTime();

  update( rootNode, state );

  int enabled = 0, disabled = 0, unloaded = 0;

  Q_FOREACH ( ChunkNode *node, activeNodes )
  {
    if ( activeBefore.contains( node ) )
      activeBefore.remove( node );
    else
    {
      node->entity->setEnabled( true );
      ++enabled;
    }
  }

  // disable those that were active but will not be anymore
  Q_FOREACH ( ChunkNode *node, activeBefore )
  {
    node->entity->setEnabled( false );
    ++disabled;
  }

  // unload those that are over the limit for replacement
  // TODO: what to do when our cache is too small and nodes are being constantly evicted + loaded again
  while ( replacementQueue->count() > maxLoadedChunks )
  {
    ChunkListEntry *entry = replacementQueue->takeLast();
    entry->chunk->unloadChunk();  // also deletes the entry
    ++unloaded;
  }

  if ( bboxesEntity )
  {
    QList<AABB> bboxes;
    Q_FOREACH ( ChunkNode *n, activeNodes )
      bboxes << n->bbox;
    bboxesEntity->setBoxes( bboxes );
  }

  // start a job from queue if there is anything waiting
  if ( !activeJob )
    startJob();

  needsUpdate = false;  // just updated

  qDebug() << "update: active " << activeNodes.count() << " enabled " << enabled << " disabled " << disabled << " | culled " << frustumCulled << " | loading " << chunkLoaderQueue->count() << " loaded " << replacementQueue->count() << " | unloaded " << unloaded << " elapsed " << t.elapsed() << "ms";
}

void ChunkedEntity::setShowBoundingBoxes( bool enabled )
{
  if ( ( enabled && bboxesEntity ) || ( !enabled && !bboxesEntity ) )
    return;

  if ( enabled )
  {
    bboxesEntity = new ChunkBoundsEntity( this );
  }
  else
  {
    bboxesEntity->deleteLater();
    bboxesEntity = nullptr;
  }
}

void ChunkedEntity::updateNodes( const QList<ChunkNode *> &nodes, ChunkQueueJobFactory *updateJobFactory )
{
  Q_FOREACH ( ChunkNode *node, nodes )
  {
    if ( node->state == ChunkNode::QueuedForUpdate )
    {
      chunkLoaderQueue->takeEntry( node->loaderQueueEntry );
      node->cancelQueuedForUpdate();
    }
    else if ( node->state == ChunkNode::Updating )
    {
      cancelActiveJob();  // we have currently just one active job so that must be it
    }

    Q_ASSERT( node->state == ChunkNode::Loaded );

    ChunkListEntry *entry = new ChunkListEntry( node );
    node->setQueuedForUpdate( entry, updateJobFactory );
    chunkLoaderQueue->insertLast( entry );
  }

  // trigger update
  if ( !activeJob )
    startJob();
}


void ChunkedEntity::update( ChunkNode *node, const SceneState &state )
{
  if ( Qgs3DUtils::isCullable( node->bbox, state.viewProjectionMatrix ) )
  {
    ++frustumCulled;
    return;
  }

  node->ensureAllChildrenExist();

  // make sure all nodes leading to children are always loaded
  // so that zooming out does not create issues
  requestResidency( node );

  if ( !node->entity )
  {
    // this happens initially when root node is not ready yet
    qDebug() << "BOOM!";
    return;
  }

  //qDebug() << node->x << "|" << node->y << "|" << node->z << "  " << tau << "  " << screenSpaceError(node, state);

  if ( screenSpaceError( node, state ) <= tau )
  {
    // acceptable error for the current chunk - let's render it

    activeNodes << node;
  }
  else if ( node->allChildChunksResident( currentTime ) )
  {
    // error is not acceptable and children are ready to be used - recursive descent

    for ( int i = 0; i < 4; ++i )
      update( node->children[i], state );
  }
  else
  {
    // error is not acceptable but children are not ready either - still use parent but request children

    activeNodes << node;

    if ( node->level() < maxLevel )
    {
      for ( int i = 0; i < 4; ++i )
        requestResidency( node->children[i] );
    }
  }
}


void ChunkedEntity::requestResidency( ChunkNode *node )
{
  if ( node->state == ChunkNode::Loaded || node->state == ChunkNode::QueuedForUpdate || node->state == ChunkNode::Updating )
  {
    Q_ASSERT( node->replacementQueueEntry );
    Q_ASSERT( node->entity );
    replacementQueue->takeEntry( node->replacementQueueEntry );
    replacementQueue->insertFirst( node->replacementQueueEntry );
  }
  else if ( node->state == ChunkNode::QueuedForLoad )
  {
    // move to the front of loading queue
    Q_ASSERT( node->loaderQueueEntry );
    Q_ASSERT( !node->loader );
    if ( node->loaderQueueEntry->prev || node->loaderQueueEntry->next )
    {
      chunkLoaderQueue->takeEntry( node->loaderQueueEntry );
      chunkLoaderQueue->insertFirst( node->loaderQueueEntry );
    }
  }
  else if ( node->state == ChunkNode::Loading )
  {
    // the entry is being currently processed - nothing to do really
  }
  else if ( node->state == ChunkNode::Skeleton )
  {
    // add to the loading queue
    ChunkListEntry *entry = new ChunkListEntry( node );
    node->setQueuedForLoad( entry );
    chunkLoaderQueue->insertFirst( entry );
  }
  else
    Q_ASSERT( false && "impossible!" );
}


void ChunkedEntity::onActiveJobFinished()
{
  ChunkQueueJob *job = qobject_cast<ChunkQueueJob *>( sender() );
  Q_ASSERT( job );
  Q_ASSERT( job == activeJob );

  ChunkNode *node = job->chunk();

  if ( ChunkLoader *loader = qobject_cast<ChunkLoader *>( job ) )
  {
    Q_ASSERT( node->state == ChunkNode::Loading );
    Q_ASSERT( node->loader == loader );

    // mark as loaded + create entity
    Qt3DCore::QEntity *entity = node->loader->createEntity( this );

    // load into node (should be in main thread again)
    node->setLoaded( entity );

    replacementQueue->insertFirst( node->replacementQueueEntry );

    // now we need an update!
    needsUpdate = true;
  }
  else
  {
    Q_ASSERT( node->state == ChunkNode::Updating );
    node->setUpdated();
  }

  // cleanup the job that has just finished
  activeJob->deleteLater();
  activeJob = nullptr;

  // start another job - if any
  startJob();
}

void ChunkedEntity::startJob()
{
  Q_ASSERT( !activeJob );
  if ( chunkLoaderQueue->isEmpty() )
    return;

  ChunkListEntry *entry = chunkLoaderQueue->takeFirst();
  Q_ASSERT( entry );
  ChunkNode *node = entry->chunk;
  delete entry;

  if ( node->state == ChunkNode::QueuedForLoad )
  {
    ChunkLoader *loader = chunkLoaderFactory->createChunkLoader( node );
    connect( loader, &ChunkQueueJob::finished, this, &ChunkedEntity::onActiveJobFinished );
    node->setLoading( loader );
    activeJob = loader;
  }
  else if ( node->state == ChunkNode::QueuedForUpdate )
  {
    node->setUpdating();
    connect( node->updater, &ChunkQueueJob::finished, this, &ChunkedEntity::onActiveJobFinished );
    activeJob = node->updater;
  }
  else
    Q_ASSERT( false );  // not possible
}

void ChunkedEntity::cancelActiveJob()
{
  Q_ASSERT( activeJob );

  ChunkNode *node = activeJob->chunk();

  if ( qobject_cast<ChunkLoader *>( activeJob ) )
  {
    // return node back to skeleton
    node->cancelLoading();
  }
  else
  {
    // return node back to loaded state
    node->cancelUpdating();
  }

  activeJob->cancel();
  activeJob->deleteLater();
  activeJob = nullptr;
}
