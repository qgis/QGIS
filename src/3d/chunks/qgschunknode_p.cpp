#include "qgschunknode_p.h"

#include "qgschunkedentity_p.h"  // for ChunkLoader destructor
#include "qgschunklist_p.h"
#include "qgschunkloader_p.h"
#include <Qt3DCore/QEntity>

///@cond PRIVATE

QgsChunkNode::QgsChunkNode( int x, int y, int z, const QgsAABB &bbox, float error, QgsChunkNode *parent )
  : bbox( bbox )
  , error( error )
  , x( x )
  , y( y )
  , z( z )
  , parent( parent )
  , state( Skeleton )
  , loaderQueueEntry( nullptr )
  , replacementQueueEntry( nullptr )
  , loader( nullptr )
  , entity( nullptr )
  , updaterFactory( nullptr )
  , updater( nullptr )
{
  for ( int i = 0; i < 4; ++i )
    children[i] = nullptr;
}

QgsChunkNode::~QgsChunkNode()
{
  Q_ASSERT( state == Skeleton );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !replacementQueueEntry );
  Q_ASSERT( !loader ); // should be deleted when removed from loader queue
  Q_ASSERT( !entity ); // should be deleted when removed from replacement queue
  Q_ASSERT( !updater );
  Q_ASSERT( !updaterFactory );
  for ( int i = 0; i < 4; ++i )
    delete children[i];
}

bool QgsChunkNode::allChildChunksResident( const QTime &currentTime ) const
{
  for ( int i = 0; i < 4; ++i )
  {
    if ( !children[i] )
      return false;  // not even a skeleton
    if ( !children[i]->entity )
      return false;  // no there yet
    Q_UNUSED( currentTime ); // seems we do not need this extra time (it just brings extra problems)
    //if (children[i]->entityCreatedTime.msecsTo(currentTime) < 100)
    //  return false;  // allow some time for upload of stuff within Qt3D (TODO: better way to check it is ready?)
  }
  return true;
}

void QgsChunkNode::ensureAllChildrenExist()
{
  float childError = error / 2;
  float xc = bbox.xCenter(), zc = bbox.zCenter();
  float ymin = bbox.yMin;
  float ymax = bbox.yMax;

  if ( !children[0] )
    children[0] = new QgsChunkNode( x * 2 + 0, y * 2 + 1, z + 1, QgsAABB( bbox.xMin, ymin, bbox.zMin, xc, ymax, zc ), childError, this );

  if ( !children[1] )
    children[1] = new QgsChunkNode( x * 2 + 0, y * 2 + 0, z + 1, QgsAABB( bbox.xMin, ymin, zc, xc, ymax, bbox.zMax ), childError, this );

  if ( !children[2] )
    children[2] = new QgsChunkNode( x * 2 + 1, y * 2 + 1, z + 1, QgsAABB( xc, ymin, bbox.zMin, bbox.xMax, ymax, zc ), childError, this );

  if ( !children[3] )
    children[3] = new QgsChunkNode( x * 2 + 1, y * 2 + 0, z + 1, QgsAABB( xc, ymin, zc, bbox.xMax, ymax, bbox.zMax ), childError, this );
}

int QgsChunkNode::level() const
{
  int lvl = 0;
  QgsChunkNode *p = parent;
  while ( p )
  {
    ++lvl;
    p = p->parent;
  }
  return lvl;
}

QList<QgsChunkNode *> QgsChunkNode::descendants()
{
  QList<QgsChunkNode *> lst;
  lst << this;

  for ( int i = 0; i < 4; ++i )
  {
    if ( children[i] )
      lst << children[i]->descendants();
  }

  return lst;
}

void QgsChunkNode::setQueuedForLoad( QgsChunkListEntry *entry )
{
  Q_ASSERT( state == Skeleton );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !loader );

  state = QgsChunkNode::QueuedForLoad;
  loaderQueueEntry = entry;
}

void QgsChunkNode::cancelQueuedForLoad()
{
  Q_ASSERT( state == QueuedForLoad );
  Q_ASSERT( loaderQueueEntry );

  delete loaderQueueEntry;
  loaderQueueEntry = nullptr;

  state = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setLoading( QgsChunkLoader *chunkLoader )
{
  Q_ASSERT( state == QueuedForLoad );
  Q_ASSERT( !loader );
  Q_ASSERT( loaderQueueEntry );

  state = Loading;
  loader = chunkLoader;
  loaderQueueEntry = nullptr;
}

void QgsChunkNode::cancelLoading()
{
  Q_ASSERT( state == QgsChunkNode::Loading );
  Q_ASSERT( loader );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !entity );
  Q_ASSERT( !replacementQueueEntry );

  loader = nullptr;  // not owned by chunk node

  state = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setLoaded( Qt3DCore::QEntity *newEntity )
{
  Q_ASSERT( state == QgsChunkNode::Loading );
  Q_ASSERT( loader );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !replacementQueueEntry );

  entity = newEntity;
  entityCreatedTime = QTime::currentTime();

  loader = nullptr;  // not owned by chunk node

  state = QgsChunkNode::Loaded;
  replacementQueueEntry = new QgsChunkListEntry( this );
}

void QgsChunkNode::unloadChunk()
{
  Q_ASSERT( state == QgsChunkNode::Loaded );
  Q_ASSERT( entity );
  Q_ASSERT( replacementQueueEntry );

  entity->deleteLater();
  entity = nullptr;
  delete replacementQueueEntry;
  replacementQueueEntry = nullptr;
  state = QgsChunkNode::Skeleton;
}

void QgsChunkNode::setQueuedForUpdate( QgsChunkListEntry *entry, QgsChunkQueueJobFactory *updateJobFactory )
{
  Q_ASSERT( state == QgsChunkNode::Loaded );
  Q_ASSERT( entity );
  Q_ASSERT( replacementQueueEntry );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !updater );
  Q_ASSERT( !updaterFactory );

  state = QueuedForUpdate;
  loaderQueueEntry = entry;
  updaterFactory = updateJobFactory;
}

void QgsChunkNode::cancelQueuedForUpdate()
{
  Q_ASSERT( state == QueuedForUpdate );
  Q_ASSERT( entity );
  Q_ASSERT( loaderQueueEntry );
  Q_ASSERT( updaterFactory );
  Q_ASSERT( !updater );

  state = Loaded;
  updaterFactory = nullptr;  // not owned by the node

  delete loaderQueueEntry;
  loaderQueueEntry = nullptr;
}

void QgsChunkNode::setUpdating()
{
  Q_ASSERT( state == QgsChunkNode::QueuedForUpdate );
  Q_ASSERT( entity );
  Q_ASSERT( replacementQueueEntry );
  Q_ASSERT( loaderQueueEntry );
  Q_ASSERT( !updater );
  Q_ASSERT( updaterFactory );

  state = Updating;
  updater = updaterFactory->createJob( this );
  updaterFactory = nullptr;  // not owned by the node
  loaderQueueEntry = nullptr;
}

void QgsChunkNode::cancelUpdating()
{
  Q_ASSERT( state == QgsChunkNode::Updating );
  Q_ASSERT( updater );
  Q_ASSERT( !loaderQueueEntry );

  updater = nullptr;  // not owned by chunk node

  state = Loaded;
}

void QgsChunkNode::setUpdated()
{
  Q_ASSERT( state == QgsChunkNode::Updating );
  Q_ASSERT( updater );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( replacementQueueEntry );

  updater = nullptr;   // not owned by chunk node

  state = QgsChunkNode::Loaded;
}

void QgsChunkNode::setExactBbox( const QgsAABB &box )
{
  bbox = box;

  // TODO: propagate better estimate to children?
}

/// @endcond
