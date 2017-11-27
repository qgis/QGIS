#include "quantizedmeshterraingenerator.h"

#include "map3d.h"
#include "quantizedmeshgeometry.h"
#include "terrain.h"

#include "qgsmapsettings.h"

#include <Qt3DRender/QGeometryRenderer>

#include "chunknode.h"
#include "terrainchunkloader.h"

/// @cond PRIVATE

//! Chunk loader for quantized mesh terrain
class QuantizedMeshTerrainChunkLoader : public TerrainChunkLoader
{
  public:
    QuantizedMeshTerrainChunkLoader( Terrain *terrain, ChunkNode *node )
      : TerrainChunkLoader( terrain, node )
      , qmt( nullptr )
    {
      const Map3D &map = mTerrain->map3D();
      QuantizedMeshTerrainGenerator *generator = static_cast<QuantizedMeshTerrainGenerator *>( map.terrainGenerator.get() );

      generator->quadTreeTileToBaseTile( node->x, node->y, node->z, tx, ty, tz );
      tileRect = map.terrainGenerator->terrainTilingScheme.tileToExtent( tx, ty, tz );

      // we need map settings here for access to mapToPixel
      mapSettings.setLayers( map.layers() );
      mapSettings.setOutputSize( QSize( map.tileTextureSize, map.tileTextureSize ) );
      mapSettings.setDestinationCrs( map.crs );
      mapSettings.setExtent( mTerrain->terrainToMapTransform().transformBoundingBox( tileRect ) );
    }

    virtual void load() override
    {
      QuantizedMeshGeometry::downloadTileIfMissing( tx, ty, tz );
      qmt = QuantizedMeshGeometry::readTile( tx, ty, tz, tileRect );
      Q_ASSERT( qmt );

      loadTexture();
    }

    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent )
    {
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      // create geometry renderer

      Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
      mesh->setGeometry( new QuantizedMeshGeometry( qmt, mTerrain->map3D(), mapSettings.mapToPixel(), mTerrain->terrainToMapTransform(), mesh ) );
      entity->addComponent( mesh );

      // create material

      createTextureComponent( entity );

      // create transform

      Qt3DCore::QTransform *transform = nullptr;
      transform = new Qt3DCore::QTransform();
      entity->addComponent( transform );

      const Map3D &map = mTerrain->map3D();

      transform->setScale3D( QVector3D( 1.f, map.zExaggeration, 1.f ) );

      QgsRectangle mapExtent = mapSettings.extent();
      float x0 = mapExtent.xMinimum() - map.originX;
      float y0 = mapExtent.yMinimum() - map.originY;
      float x1 = mapExtent.xMaximum() - map.originX;
      float y1 = mapExtent.yMaximum() - map.originY;
      float z0 = qmt->header.MinimumHeight, z1 = qmt->header.MaximumHeight;

      node->setExactBbox( AABB( x0, z0 * map.zExaggeration, -y0, x1, z1 * map.zExaggeration, -y1 ) );
      //epsilon = mapExtent.width() / map.tileTextureSize;

      entity->setEnabled( false );
      entity->setParent( parent );
      return entity;
    }

  protected:
    QuantizedMeshTile *qmt = nullptr;
    QgsMapSettings mapSettings;
    int tx, ty, tz;
    QgsRectangle tileRect;
};

/// @endcond


// ---------------



QuantizedMeshTerrainGenerator::QuantizedMeshTerrainGenerator()
{
  terrainBaseX = terrainBaseY = terrainBaseZ = 0;
  terrainTilingScheme = TilingScheme( QgsRectangle( -180, -90, 0, 90 ), QgsCoordinateReferenceSystem( "EPSG:4326" ) );
}

void QuantizedMeshTerrainGenerator::setBaseTileFromExtent( const QgsRectangle &extentInTerrainCrs )
{
  terrainTilingScheme.extentToTile( extentInTerrainCrs, terrainBaseX, terrainBaseY, terrainBaseZ );
}

void QuantizedMeshTerrainGenerator::quadTreeTileToBaseTile( int x, int y, int z, int &tx, int &ty, int &tz ) const
{
  // true tile coords (using the base tile pos)
  int multiplier = pow( 2, z );
  tx = terrainBaseX * multiplier + x;
  ty = terrainBaseY * multiplier + y;
  tz = terrainBaseZ + z;
}

TerrainGenerator::Type QuantizedMeshTerrainGenerator::type() const
{
  return TerrainGenerator::QuantizedMesh;
}

QgsRectangle QuantizedMeshTerrainGenerator::extent() const
{
  return terrainTilingScheme.tileToExtent( terrainBaseX, terrainBaseY, terrainBaseZ );
}

void QuantizedMeshTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "base-x", terrainBaseX );
  elem.setAttribute( "base-y", terrainBaseY );
  elem.setAttribute( "base-z", terrainBaseZ );
}

void QuantizedMeshTerrainGenerator::readXml( const QDomElement &elem )
{
  terrainBaseX = elem.attribute( "base-x" ).toInt();
  terrainBaseY = elem.attribute( "base-y" ).toInt();
  terrainBaseZ = elem.attribute( "base-z" ).toInt();
  // TODO: update tiling scheme
}

ChunkLoader *QuantizedMeshTerrainGenerator::createChunkLoader( ChunkNode *node ) const
{
  return new QuantizedMeshTerrainChunkLoader( mTerrain, node );
}
