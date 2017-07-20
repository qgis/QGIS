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
  for ( int i = 0; i < 4; ++i )
    delete children[i];
}

bool ChunkNode::allChildChunksResident( const QTime &currentTime ) const
{
  for ( int i = 0; i < 4; ++i )
  {
    if ( !children[i] )
      return false;  // not even a skeleton
    if ( children[i]->state != Loaded )
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

void ChunkNode::setLoading( ChunkLoader *chunkLoader, ChunkListEntry *entry )
{
  Q_ASSERT( !loaderQueueEntry );
  Q_ASSERT( !loader );

  state = ChunkNode::Loading;
  loader = chunkLoader;
  loaderQueueEntry = entry;
}

void ChunkNode::setLoaded( Qt3DCore::QEntity *newEntity, ChunkListEntry *entry )
{
  Q_ASSERT( state == ChunkNode::Loading );
  Q_ASSERT( loader );

  entity = newEntity;
  entityCreatedTime = QTime::currentTime();

  delete loader;
  loader = nullptr;

  state = ChunkNode::Loaded;
  loaderQueueEntry = nullptr;
  replacementQueueEntry = entry;
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

void ChunkNode::setExactBbox( const AABB &box )
{
  bbox = box;

  // TODO: propagate better estimate to children?
}
