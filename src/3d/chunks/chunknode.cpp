#include "chunknode.h"

#include "chunkedentity.h"  // for ChunkLoader destructor
#include "chunklist.h"
#include "chunkloader.h"
#include <Qt3DCore/QEntity>


ChunkNode::ChunkNode( int x, int y, int z, const AABB &bbox, float error, ChunkNode *parent )
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

ChunkNode::~ChunkNode()
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

bool ChunkNode::allChildChunksResident( const QTime &currentTime ) const
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

void ChunkNode::ensureAllChildrenExist()
{
  float childError = error / 2;
  float xc = bbox.xCenter(), zc = bbox.zCenter();
  float ymin = bbox.yMin;
  float ymax = bbox.yMax;

  if ( !children[0] )
    children[0] = new ChunkNode( x * 2 + 0, y * 2 + 1, z + 1, AABB( bbox.xMin, ymin, bbox.zMin, xc, ymax, zc ), childError, this );

  if ( !children[1] )
    children[1] = new ChunkNode( x * 2 + 0, y * 2 + 0, z + 1, AABB( bbox.xMin, ymin, zc, xc, ymax, bbox.zMax ), childError, this );

  if ( !children[2] )
    children[2] = new ChunkNode( x * 2 + 1, y * 2 + 1, z + 1, AABB( xc, ymin, bbox.zMin, bbox.xMax, ymax, zc ), childError, this );

  if ( !children[3] )
    children[3] = new ChunkNode( x * 2 + 1, y * 2 + 0, z + 1, AABB( xc, ymin, zc, bbox.xMax, ymax, bbox.zMax ), childError, this );
}

int ChunkNode::level() const
{
  int lvl = 0;
  ChunkNode *p = parent;
  while ( p )
  {
    ++lvl;
    p = p->parent;
  }
  return lvl;
}

QList<ChunkNode *> ChunkNode::descendants()
{
  QList<ChunkNode *> lst;
  lst << this;

  for ( int i = 0; i < 4; ++i )
  {
    if ( children[i] )
      lst << children[i]->descendants();
  }

  return lst;
}

void ChunkNode::setQueuedForLoad( ChunkListEntry *entry )
{
  Q_ASSERT( state == Skeleton );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !loader );

  state = ChunkNode::QueuedForLoad;
  loaderQueueEntry = entry;
}

void ChunkNode::cancelQueuedForLoad()
{
  Q_ASSERT( state == QueuedForLoad );
  Q_ASSERT( loaderQueueEntry );

  delete loaderQueueEntry;
  loaderQueueEntry = nullptr;

  state = ChunkNode::Skeleton;
}

void ChunkNode::setLoading( ChunkLoader *chunkLoader )
{
  Q_ASSERT( state == QueuedForLoad );
  Q_ASSERT( !loader );
  Q_ASSERT( loaderQueueEntry );

  state = Loading;
  loader = chunkLoader;
  loaderQueueEntry = nullptr;
}

void ChunkNode::cancelLoading()
{
  Q_ASSERT( state == ChunkNode::Loading );
  Q_ASSERT( loader );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !entity );
  Q_ASSERT( !replacementQueueEntry );

  loader = nullptr;  // not owned by chunk node

  state = ChunkNode::Skeleton;
}

void ChunkNode::setLoaded( Qt3DCore::QEntity *newEntity )
{
  Q_ASSERT( state == ChunkNode::Loading );
  Q_ASSERT( loader );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !replacementQueueEntry );

  entity = newEntity;
  entityCreatedTime = QTime::currentTime();

  loader = nullptr;  // not owned by chunk node

  state = ChunkNode::Loaded;
  replacementQueueEntry = new ChunkListEntry( this );
}

void ChunkNode::unloadChunk()
{
  Q_ASSERT( state == ChunkNode::Loaded );
  Q_ASSERT( entity );
  Q_ASSERT( replacementQueueEntry );

  entity->deleteLater();
  entity = nullptr;
  delete replacementQueueEntry;
  replacementQueueEntry = nullptr;
  state = ChunkNode::Skeleton;
}

void ChunkNode::setQueuedForUpdate( ChunkListEntry *entry, ChunkQueueJobFactory *updateJobFactory )
{
  Q_ASSERT( state == ChunkNode::Loaded );
  Q_ASSERT( entity );
  Q_ASSERT( replacementQueueEntry );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !updater );
  Q_ASSERT( !updaterFactory );

  state = QueuedForUpdate;
  loaderQueueEntry = entry;
  updaterFactory = updateJobFactory;
}

void ChunkNode::cancelQueuedForUpdate()
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

void ChunkNode::setUpdating()
{
  Q_ASSERT( state == ChunkNode::QueuedForUpdate );
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

void ChunkNode::cancelUpdating()
{
  Q_ASSERT( state == ChunkNode::Updating );
  Q_ASSERT( updater );
  Q_ASSERT( !loaderQueueEntry );

  updater = nullptr;  // not owned by chunk node

  state = Loaded;
}

void ChunkNode::setUpdated()
{
  Q_ASSERT( state == ChunkNode::Updating );
  Q_ASSERT( updater );
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( replacementQueueEntry );

  updater = nullptr;   // not owned by chunk node

  state = ChunkNode::Loaded;
}

void ChunkNode::setExactBbox( const AABB &box )
{
  bbox = box;

  // TODO: propagate better estimate to children?
}
