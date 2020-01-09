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

  auto entity = new QgsMesh3dTerrainTileEntity( terrain()->map3D(), mMeshLayer, mSymbol, mNode->tileId(), parent );
  entity->build();

  createTexture( entity );

  return entity;
}


QgsChunkLoader *QgsMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsMeshTerrainTileLoader( mTerrain, node, meshLayer(), symbol() );
}

void QgsMeshTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  updateGenerator();
}

void QgsMeshTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
}

QgsMeshLayer *QgsMeshTerrainGenerator::meshLayer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayer.layer.data() );
}

QgsTerrainGenerator *QgsMeshTerrainGenerator::clone() const
{
  QgsMeshTerrainGenerator *cloned = new QgsMeshTerrainGenerator();
  cloned->mLayer = mLayer;
  cloned->mTerrainTilingScheme = QgsTilingScheme();
  return cloned;
}

QgsTerrainGenerator::Type QgsMeshTerrainGenerator::type() const {return QgsTerrainGenerator::Mesh;}

QgsRectangle QgsMeshTerrainGenerator::extent() const
{
  if ( mLayer )
    return mLayer->extent();
  else
    return QgsRectangle();
}

QgsMesh3DSymbol QgsMeshTerrainGenerator::symbol() const
{
  return mSymbol;
}

void QgsMeshTerrainGenerator::setSymbol( const QgsMesh3DSymbol &symbol )
{
  mSymbol = symbol;
}
