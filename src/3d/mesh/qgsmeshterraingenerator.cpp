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


QgsMeshTerrainTileLoader::QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, const QgsTriangularMesh &triangularMesh, const QgsRectangle &extent, const QgsMesh3DSymbol &symbol ):
  QgsTerrainTileLoader( terrain, node ),
  mExtent( extent ),
  mTriangularMesh( triangularMesh ),
  mSymbol( symbol )
{
  loadTexture();
}

Qt3DCore::QEntity *QgsMeshTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  auto entity = new QgsMesh3dTerrainTileEntity( terrain()->map3D(), mTriangularMesh, mExtent, mSymbol, mNode->tileId(), parent );
  entity->build();
  createTexture( entity );

  return entity;
}

QgsChunkLoader *QgsMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  if ( !mLayer )
    return nullptr;

  QgsCoordinateTransform terrainToMapTransform( mLayer->crs(), mCrs, mTransformContext );

  meshLayer()->updateTriangularMesh( terrainToMapTransform );
  return new QgsMeshTerrainTileLoader( mTerrain, node, *meshLayer()->triangularMesh(), extent(), symbol() );
}

float QgsMeshTerrainGenerator::rootChunkError( const Qgs3DMapSettings &map ) const {Q_UNUSED( map ); return 0;}

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
    float zValue = triangularMesh->vertices().at( i ).z();
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
  cloned->mSymbol = mSymbol;
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
  catch ( QgsCsException &e )
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
  mSymbol.writeXml( elemSymbol, rwc );
  elem.appendChild( elemSymbol );
}

void QgsMeshTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
  QgsReadWriteContext rwc;
  mSymbol.readXml( elem.firstChildElement( "symbol" ), rwc );
}

QgsMesh3DSymbol QgsMeshTerrainGenerator::symbol() const
{
  return mSymbol;
}

void QgsMeshTerrainGenerator::setSymbol( const QgsMesh3DSymbol &symbol )
{
  mSymbol = symbol;
}

void QgsMeshTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
}
