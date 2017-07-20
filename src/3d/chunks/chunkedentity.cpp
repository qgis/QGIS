#include "chunkedentity.h"

#include "chunkboundsentity.h"
#include "chunklist.h"
#include "chunkloader.h"
#include "chunknode.h"


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


#include <QVector4D>

//! coarse box vs frustum test for culling.
//! corners of oriented box are transformed to clip space and new axis-aligned box is created for intersection test
static bool isInFrustum( const AABB &bbox, const QMatrix4x4 &viewProjectionMatrix )
{
  float xmin, ymin, zmin, xmax, ymax, zmax;
  for ( int i = 0; i < 8; ++i )
  {
    QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax,
                 ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax,
                 ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );
    QVector4D pc = viewProjectionMatrix * p;
    pc /= pc.w();
    float x = pc.x(), y = pc.y(), z = pc.z();

    if ( i == 0 )
    {
      xmin = xmax = x;
      ymin = ymax = y;
      zmin = zmax = z;
    }
    else
    {
      if ( x < xmin ) xmin = x;
      if ( x > xmax ) xmax = x;
      if ( y < ymin ) ymin = y;
      if ( y > ymax ) ymax = y;
      if ( z < zmin ) zmin = z;
      if ( z > zmax ) zmax = z;
    }
  }
  return AABB( -1, -1, -1, 1, 1, 1 ).intersects( AABB( xmin, ymin, zmin, xmax, ymax, zmax ) );
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

  loaderThread = new LoaderThread( chunkLoaderQueue, loaderMutex, loaderWaitCondition );
  connect( loaderThread, &LoaderThread::nodeLoaded, this, &ChunkedEntity::onNodeLoaded );
  loaderThread->start();
}


ChunkedEntity::~ChunkedEntity()
{
  loaderThread->setStopping( true );
  loaderWaitCondition.wakeOne();  // may be waiting
  loaderThread->wait();
  delete loaderThread;

  // clean up any pending load requests
  while ( !chunkLoaderQueue->isEmpty() )
  {
    ChunkListEntry *entry = chunkLoaderQueue->takeFirst();
    ChunkNode *node = entry->chunk;

    delete entry;
    delete node->loader;

    // unload node that is in "loading" state
    node->state = ChunkNode::Skeleton;
    node->loaderQueueEntry = nullptr;
    node->loader = nullptr;
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


void ChunkedEntity::update( const SceneState &state )
{
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

  needsUpdate = false;  // just updated

  qDebug() << "update: active " << activeNodes.count() << " enabled " << enabled << " disabled " << disabled << " | culled " << frustumCulled << " | loading " << chunkLoaderQueue->count() << " loaded " << replacementQueue->count() << " | unloaded " << unloaded;
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


void ChunkedEntity::update( ChunkNode *node, const SceneState &state )
{
  // TODO: fix and re-enable frustum culling
  if ( 0 && !isInFrustum( node->bbox, state.viewProjectionMatrix ) )
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
  if ( node->state == ChunkNode::Loaded )
  {
    Q_ASSERT( node->replacementQueueEntry );
    Q_ASSERT( node->entity );
    replacementQueue->takeEntry( node->replacementQueueEntry );
    replacementQueue->insertFirst( node->replacementQueueEntry );
  }
  else if ( node->state == ChunkNode::Loading )
  {
    // move to the front of loading queue
    loaderMutex.lock();
    Q_ASSERT( node->loaderQueueEntry );
    Q_ASSERT( node->loader );
    if ( node->loaderQueueEntry->prev || node->loaderQueueEntry->next )
    {
      chunkLoaderQueue->takeEntry( node->loaderQueueEntry );
      chunkLoaderQueue->insertFirst( node->loaderQueueEntry );
    }
    else
    {
      // the entry is being currently processed by the loading thread
      // (or it is at the head of 1-entry list)
    }
    loaderMutex.unlock();
  }
  else if ( node->state == ChunkNode::Skeleton )
  {
    // add to the loading queue
    loaderMutex.lock();
    ChunkListEntry *entry = new ChunkListEntry( node );
    node->setLoading( chunkLoaderFactory->createChunkLoader( node ), entry );
    chunkLoaderQueue->insertFirst( entry );
    if ( chunkLoaderQueue->count() == 1 )
      loaderWaitCondition.wakeOne();
    loaderMutex.unlock();
  }
  else
    Q_ASSERT( false && "impossible!" );
}

void ChunkedEntity::onNodeLoaded( ChunkNode *node )
{
  Qt3DCore::QEntity *entity = node->loader->createEntity( this );

  loaderMutex.lock();
  ChunkListEntry *entry = node->loaderQueueEntry;

  // load into node (should be in main thread again)
  node->setLoaded( entity, entry );
  loaderMutex.unlock();

  replacementQueue->insertFirst( entry );

  // now we need an update!
  needsUpdate = true;
}


// -------


LoaderThread::LoaderThread( ChunkList *list, QMutex &mutex, QWaitCondition &waitCondition )
  : loadList( list )
  , mutex( mutex )
  , waitCondition( waitCondition )
  , stopping( false )
{
}

void LoaderThread::run()
{
  while ( 1 )
  {
    ChunkListEntry *entry = nullptr;
    mutex.lock();
    if ( loadList->isEmpty() )
      waitCondition.wait( &mutex );

    // we can get woken up also when we need to stop
    if ( stopping )
    {
      mutex.unlock();
      break;
    }

    Q_ASSERT( !loadList->isEmpty() );
    entry = loadList->takeFirst();
    mutex.unlock();

    qDebug() << "[THR] loading! " << entry->chunk->x << " | " << entry->chunk->y << " | " << entry->chunk->z;

    entry->chunk->loader->load();

    qDebug() << "[THR] done!";

    emit nodeLoaded( entry->chunk );

    if ( stopping )
    {
      // this chunk we just processed will not be processed anymore because we are shutting down everything
      // so at least put it back into the loader queue so that we can clean up the chunk
      loadList->insertFirst( entry );
    }
  }
}
