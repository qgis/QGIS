#include "terrainchunkloader.h"

#include "chunknode.h"
#include "maptextureimage.h"
#include "maptexturegenerator.h"
#include "map3d.h"
#include "terrain.h"
#include "terraingenerator.h"

#include <Qt3DRender/QTexture>

#if QT_VERSION >= 0x050900
#include <Qt3DExtras/QTextureMaterial>
#else
#include <Qt3DExtras/QDiffuseMapMaterial>
#endif

#include "quantizedmeshterraingenerator.h"


TerrainChunkLoader::TerrainChunkLoader( Terrain *terrain, ChunkNode *node )
  : ChunkLoader( node )
  , mTerrain( terrain )
{
  const Map3D &map = mTerrain->map3D();
  int tx, ty, tz;
#if 0
  if ( map.terrainGenerator->type() == TerrainGenerator::QuantizedMesh )
  {
    // TODO: sort out - should not be here
    QuantizedMeshTerrainGenerator *generator = static_cast<QuantizedMeshTerrainGenerator *>( map.terrainGenerator.get() );
    generator->quadTreeTileToBaseTile( node->x, node->y, node->z, tx, ty, tz );
  }
  else
#endif
  {
    tx = node->x;
    ty = node->y;
    tz = node->z;
  }

  QgsRectangle extentTerrainCrs = map.terrainGenerator->terrainTilingScheme.tileToExtent( tx, ty, tz );
  mExtentMapCrs = terrain->terrainToMapTransform().transformBoundingBox( extentTerrainCrs );
  mTileDebugText = map.drawTerrainTileInfo ? QString( "%1 | %2 | %3" ).arg( tx ).arg( ty ).arg( tz ) : QString();
}

void TerrainChunkLoader::loadTexture()
{
  mTextureImage = mTerrain->mapTextureGenerator()->renderSynchronously( mExtentMapCrs, mTileDebugText );
}

void TerrainChunkLoader::createTextureComponent( Qt3DCore::QEntity *entity )
{
  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D( entity );
  MapTextureImage *image = new MapTextureImage( mTextureImage, mExtentMapCrs, mTileDebugText );
  texture->addTextureImage( image );
  texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  Qt3DExtras::QTextureMaterial *material;
#if QT_VERSION >= 0x050900
  material = new Qt3DExtras::QTextureMaterial;
  material->setTexture( texture );
#else
  material = new Qt3DExtras::QDiffuseMapMaterial;
  material->setDiffuse( texture );
  material->setShininess( 1 );
  material->setAmbient( Qt::white );
#endif
  entity->addComponent( material ); // takes ownership if the component has no parent
}
