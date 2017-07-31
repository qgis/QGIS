#include "terrain.h"

#include "aabb.h"
#include "chunknode.h"
#include "map3d.h"
#include "maptexturegenerator.h"
#include "maptextureimage.h"
#include "terraingenerator.h"

#include "qgscoordinatetransform.h"

#include <Qt3DRender/QObjectPicker>



class TerrainMapUpdateJobFactory : public ChunkQueueJobFactory
{
  public:
    TerrainMapUpdateJobFactory( MapTextureGenerator *mapTextureGenerator )
      : mMapTextureGenerator( mapTextureGenerator )
    {
    }

    virtual ChunkQueueJob *createJob( ChunkNode *chunk )
    {
      return new TerrainMapUpdateJob( mMapTextureGenerator, chunk );
    }

  private:
    MapTextureGenerator *mMapTextureGenerator;
};





Terrain::Terrain( int maxLevel, const Map3D &map, Qt3DCore::QNode *parent )
  : ChunkedEntity( map.terrainGenerator()->rootChunkBbox( map ),
                   map.terrainGenerator()->rootChunkError( map ),
                   map.maxTerrainScreenError(), maxLevel, map.terrainGenerator(), parent )
  , map( map )
  , mTerrainPicker( nullptr )
{
  map.terrainGenerator()->setTerrain( this );

  connect( &map, &Map3D::showTerrainBoundingBoxesChanged, this, &Terrain::onShowBoundingBoxesChanged );
  connect( &map, &Map3D::showTerrainTilesInfoChanged, this, &Terrain::invalidateMapImages );
  connect( &map, &Map3D::layersChanged, this, &Terrain::invalidateMapImages );

  mTerrainToMapTransform = new QgsCoordinateTransform( map.terrainGenerator()->crs(), map.crs );

  mMapTextureGenerator = new MapTextureGenerator( map );

  mUpdateJobFactory.reset( new TerrainMapUpdateJobFactory( mMapTextureGenerator ) );

  mTerrainPicker = new Qt3DRender::QObjectPicker;
  // add camera control's terrain picker as a component to be able to capture height where mouse was
  // pressed in order to correcly pan camera when draggin mouse
  addComponent( mTerrainPicker );
}

Terrain::~Terrain()
{
  // cancel / wait for jobs
  if ( activeJob )
    cancelActiveJob();

  delete mMapTextureGenerator;
  delete mTerrainToMapTransform;
}

void Terrain::onShowBoundingBoxesChanged()
{
  setShowBoundingBoxes( map.showTerrainBoundingBoxes() );
}


void Terrain::invalidateMapImages()
{
  qDebug() << "TERRAIN - INVALIDATE MAP IMAGES";

  // handle active nodes

  updateNodes( activeNodes, mUpdateJobFactory.get() );
  qDebug() << " updating " << activeNodes.count() << " active nodes";

  // handle inactive nodes afterwards

  QList<ChunkNode *> inactiveNodes;
  Q_FOREACH ( ChunkNode *node, rootNode->descendants() )
  {
    if ( !node->entity )
      continue;
    if ( activeNodes.contains( node ) )
      continue;
    inactiveNodes << node;
  }

  updateNodes( inactiveNodes, mUpdateJobFactory.get() );
  qDebug() << " updating " << inactiveNodes.count() << " inactive nodes";

  needsUpdate = true;
}


// -----------


TerrainMapUpdateJob::TerrainMapUpdateJob( MapTextureGenerator *mapTextureGenerator, ChunkNode *node )
  : ChunkQueueJob( node )
  , mMapTextureGenerator( mapTextureGenerator )
{
  TerrainChunkEntity *entity = qobject_cast<TerrainChunkEntity *>( node->entity );
  connect( mapTextureGenerator, &MapTextureGenerator::tileReady, this, &TerrainMapUpdateJob::onTileReady );
  mJobId = mapTextureGenerator->render( entity->mTextureImage->imageExtent(), entity->mTextureImage->imageDebugText() );
}

void TerrainMapUpdateJob::cancel()
{
  if ( mJobId != -1 )
    mMapTextureGenerator->cancelJob( mJobId );
}


void TerrainMapUpdateJob::onTileReady( int jobId, const QImage &image )
{
  if ( mJobId == jobId )
  {
    TerrainChunkEntity *entity = qobject_cast<TerrainChunkEntity *>( node->entity );
    entity->mTextureImage->setImage( image );
    mJobId = -1;
    emit finished();
  }
}
