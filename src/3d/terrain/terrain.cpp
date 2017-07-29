#include "terrain.h"

#include "aabb.h"
#include "map3d.h"
#include "maptexturegenerator.h"
#include "terraingenerator.h"

#include "qgscoordinatetransform.h"

#include <Qt3DRender/QObjectPicker>


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

  mTerrainPicker = new Qt3DRender::QObjectPicker;
  // add camera control's terrain picker as a component to be able to capture height where mouse was
  // pressed in order to correcly pan camera when draggin mouse
  addComponent( mTerrainPicker );
}

Terrain::~Terrain()
{
  delete mMapTextureGenerator;
  delete mTerrainToMapTransform;
}

void Terrain::onShowBoundingBoxesChanged()
{
  setShowBoundingBoxes( map.showTerrainBoundingBoxes() );
}

#include "maptextureimage.h"
void Terrain::invalidateMapImages()
{
  qDebug() << "!!! INVALIDATE MAP IMAGES !!!";
  QList<TerrainChunkEntity *> active, inactive;
  Q_FOREACH ( TerrainChunkEntity *entity, findChildren<TerrainChunkEntity *>() )
  {
    if ( entity->isEnabled() )
      active << entity;
    else
      inactive << entity;
    entity->mTextureImage->invalidate(); // turn into placeholder
  }

  mEntitiesToUpdate.clear();
  mEntitiesToUpdate << active << inactive;

  // TODO: do it in background

  qDebug() << "\n\nTO UPDATE:" << mEntitiesToUpdate.count() << "\n";

  Q_FOREACH ( TerrainChunkEntity *entity, mEntitiesToUpdate )
  {
    qDebug() << "-------- UPDATING ENTITY";
    QImage img = mapTextureGenerator()->renderSynchronously( entity->mTextureImage->imageExtent(), entity->mTextureImage->imageDebugText() );
    entity->mTextureImage->setImage( img );
  }
}
