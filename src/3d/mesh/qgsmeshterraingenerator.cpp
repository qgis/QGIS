#include "qgsmeshterraingenerator.h"

#include <Qt3DRender/QMaterial>

#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QTextureMaterial>

#include "qgsmesh3dentity.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsterrainentity_p.h"
#include "qgsterraintextureimage_p.h"


Qt3DCore::QEntity *QgsMeshTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( !mMeshLayer )
    return nullptr;

  QgsMeshLayer3DRenderer *renderer = static_cast<QgsMeshLayer3DRenderer *>( mMeshLayer->renderer3D() );
  QgsMesh3DSymbol symbol;
  if ( renderer && renderer->symbol() )
    symbol = *renderer->symbol();

  auto entity = new QgsMesh3dTerrainTileEntity( terrain()->map3D(), mMeshLayer, symbol, mNode->tileId(), parent );
  entity->build();

  createTexture( entity );

  return entity;
}


QgsChunkLoader *QgsMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsMeshTerrainTileLoader( mTerrain, node, mMeshLayer );
}

void QgsMeshTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  mMeshLayer = layer;
}

QgsMeshLayer *QgsMeshTerrainGenerator::meshLayer() const
{
  return mMeshLayer;
}

QgsTerrainGenerator *QgsMeshTerrainGenerator::clone() const
{
  QgsMeshTerrainGenerator *cloned = new QgsMeshTerrainGenerator();
  cloned->mMeshLayer = mMeshLayer;
  cloned->mTerrainTilingScheme = QgsTilingScheme();
  return cloned;
}

QgsTerrainGenerator::Type QgsMeshTerrainGenerator::type() const {return QgsTerrainGenerator::Mesh;}

QgsRectangle QgsMeshTerrainGenerator::extent() const
{
  if ( mMeshLayer )
    return mMeshLayer->extent();
  else
    return QgsRectangle();
}
