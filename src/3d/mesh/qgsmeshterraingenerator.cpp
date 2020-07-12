/***************************************************************************
                         qgsmeshterraingenerator.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshterraingenerator.h"

#include <Qt3DRender/QMaterial>

#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QTextureMaterial>

#include "qgsmesh3dentity_p.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsterrainentity_p.h"
#include "qgsterraintextureimage_p.h"


QgsMeshTerrainTileLoader::QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsMeshLayer *layer, const QgsMesh3DSymbol *symbol )
  : QgsTerrainTileLoader( terrain, node )
  , mLayerRef( layer )
  , mSymbol( symbol->clone() )
{
  loadTexture();
}

Qt3DCore::QEntity *QgsMeshTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsMeshLayer *layer = qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
  if ( !layer )
    return nullptr;

  QgsCoordinateTransform transform( terrain()->map3D().crs(), layer->crs(), terrain()->map3D().transformContext() );
  layer->updateTriangularMesh( transform );
  QgsMesh3dTerrainTileEntity *entity = new QgsMesh3dTerrainTileEntity( terrain()->map3D(), layer, mSymbol.get(), mNode->tileId(), parent );
  entity->build();
  createTexture( entity );

  return entity;
}

//
// QgsMeshTerrainGenerator
//

QgsMeshTerrainGenerator::QgsMeshTerrainGenerator()
  : mSymbol( qgis::make_unique< QgsMesh3DSymbol >() )
{

}

QgsChunkLoader *QgsMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  Q_ASSERT( meshLayer() );

  return new QgsMeshTerrainTileLoader( mTerrain, node, meshLayer(), symbol() );
}

float QgsMeshTerrainGenerator::rootChunkError( const Qgs3DMapSettings & ) const
{
  return 0;
}

void QgsMeshTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  if ( !meshLayer()  || !meshLayer()->triangularMesh() )
  {
    QgsTerrainGenerator::rootChunkHeightRange( hMin, hMax );
    return;
  }

  QgsTriangularMesh *triangularMesh = meshLayer()->triangularMesh();

  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();

  for ( int i = 0; i < triangularMesh->vertices().count(); ++i )
  {
    float zValue = static_cast< float >( triangularMesh->vertices().at( i ).z() );
    if ( min > zValue )
      min = zValue;
    if ( max < zValue )
      max = zValue;
  }

  hMin = min;
  hMax = max;
}

void QgsMeshTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
}

void QgsMeshTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  mIsValid = layer != nullptr;
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
  cloned->mCrs = mCrs;
  cloned->mSymbol.reset( mSymbol->clone() );
  cloned->mTransformContext = mTransformContext;
  return cloned;
}

QgsTerrainGenerator::Type QgsMeshTerrainGenerator::type() const {return QgsTerrainGenerator::Mesh;}

QgsRectangle QgsMeshTerrainGenerator::extent() const
{
  QgsRectangle layerextent;
  if ( mLayer )
    layerextent = mLayer->extent();
  else
    return QgsRectangle();

  QgsCoordinateTransform terrainToMapTransform( mLayer->crs(), mCrs, mTransformContext );
  QgsRectangle extentInMap;

  try
  {
    extentInMap = terrainToMapTransform.transform( mLayer->extent() );
  }
  catch ( QgsCsException & )
  {
    extentInMap = mLayer->extent();
  }

  return extentInMap;
}

void QgsMeshTerrainGenerator::writeXml( QDomElement &elem ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );
  QDomElement elemSymbol = doc.createElement( "symbol" );
  QgsReadWriteContext rwc;
  mSymbol->writeXml( elemSymbol, rwc );
  elem.appendChild( elemSymbol );
}

void QgsMeshTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
  QgsReadWriteContext rwc;
  mSymbol->readXml( elem.firstChildElement( "symbol" ), rwc );
}

QgsMesh3DSymbol *QgsMeshTerrainGenerator::symbol() const
{
  return mSymbol.get();
}

void QgsMeshTerrainGenerator::setSymbol( QgsMesh3DSymbol *symbol )
{
  mSymbol.reset( symbol );
}

void QgsMeshTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
}
