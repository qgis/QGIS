#include "flatterraingenerator.h"

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPlaneGeometry>

#include "map3d.h"
#include "terrain.h"

#include "chunknode.h"
#include "terrainchunkloader.h"


class FlatTerrainChunkLoader : public TerrainChunkLoader
{
  public:
    FlatTerrainChunkLoader( Terrain *terrain, Qt3DExtras::QPlaneGeometry *tileGeometry, ChunkNode *node );

    virtual void load() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    Qt3DExtras::QPlaneGeometry *mTileGeometry;
};


//---------------


FlatTerrainChunkLoader::FlatTerrainChunkLoader( Terrain *terrain, Qt3DExtras::QPlaneGeometry *tileGeometry, ChunkNode *node )
  : TerrainChunkLoader( terrain, node )
  , mTileGeometry( tileGeometry )
{
}

void FlatTerrainChunkLoader::load()
{
  loadTexture();
}

Qt3DCore::QEntity *FlatTerrainChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  // make geometry renderer

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( mTileGeometry ); // does not take ownership - geometry is already owned by FlatTerrain entity
  entity->addComponent( mesh ); // takes ownership if the component has no parent

  // create material

  createTextureComponent( entity );

  // create transform

  Qt3DCore::QTransform *transform;
  transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );

  // set up transform according to the extent covered by the quad geometry
  AABB bbox = node->bbox;
  double side = bbox.xMax - bbox.xMin;
  double half = side / 2;

  transform->setScale( side );
  transform->setTranslation( QVector3D( bbox.xMin + half, 0, bbox.zMin + half ) );

  entity->setEnabled( false );
  entity->setParent( parent );
  return entity;
}


// ---------------


FlatTerrainGenerator::FlatTerrainGenerator()
{
  // simple quad geometry shared by all tiles
  // QPlaneGeometry by default is 1x1 with mesh resultion QSize(2,2), centered at 0
  tileGeometry = new Qt3DExtras::QPlaneGeometry;  // TODO: parent to a node...
}

ChunkLoader *FlatTerrainGenerator::createChunkLoader( ChunkNode *node ) const
{
  return new FlatTerrainChunkLoader( mTerrain, tileGeometry, node );
}

TerrainGenerator *FlatTerrainGenerator::clone() const
{
  FlatTerrainGenerator *cloned = new FlatTerrainGenerator;
  cloned->mCrs = mCrs;
  cloned->mExtent = mExtent;
  cloned->updateTilingScheme();
  return cloned;
}

TerrainGenerator::Type FlatTerrainGenerator::type() const
{
  return TerrainGenerator::Flat;
}

QgsRectangle FlatTerrainGenerator::extent() const
{
  return terrainTilingScheme.tileToExtent( 0, 0, 0 );
}

void FlatTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  hMin = 0;
  hMax = 0;
}

void FlatTerrainGenerator::writeXml( QDomElement &elem ) const
{
  QgsRectangle r = mExtent;
  QDomElement elemExtent = elem.ownerDocument().createElement( "extent" );
  elemExtent.setAttribute( "xmin", QString::number( r.xMinimum() ) );
  elemExtent.setAttribute( "xmax", QString::number( r.xMaximum() ) );
  elemExtent.setAttribute( "ymin", QString::number( r.yMinimum() ) );
  elemExtent.setAttribute( "ymax", QString::number( r.yMaximum() ) );

  // crs is not read/written - it should be the same as destination crs of the map
}

void FlatTerrainGenerator::readXml( const QDomElement &elem )
{
  QDomElement elemExtent = elem.firstChildElement( "extent" );
  double xmin = elemExtent.attribute( "xmin" ).toDouble();
  double xmax = elemExtent.attribute( "xmax" ).toDouble();
  double ymin = elemExtent.attribute( "ymin" ).toDouble();
  double ymax = elemExtent.attribute( "ymax" ).toDouble();

  setExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );

  // crs is not read/written - it should be the same as destination crs of the map
}

void FlatTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
  updateTilingScheme();
}

void FlatTerrainGenerator::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
  updateTilingScheme();
}

void FlatTerrainGenerator::updateTilingScheme()
{
  if ( mExtent.isNull() )
  {
    terrainTilingScheme = TilingScheme();
  }
  else
  {
    // the real extent will be a square where the given extent fully fits
    terrainTilingScheme = TilingScheme( mExtent, mCrs );
  }
}
