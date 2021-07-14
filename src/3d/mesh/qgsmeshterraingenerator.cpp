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

#include "qgsmesh3dentity_p.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsterrainentity_p.h"
#include "qgsterraintextureimage_p.h"
#include "qgsmeshlayerutils.h"


QgsMeshTerrainTileLoader::QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol )
  : QgsTerrainTileLoader( terrain, node )
  , mTriangularMesh( triangularMesh )
  , mSymbol( symbol->clone() )
{
  loadTexture();
}

Qt3DCore::QEntity *QgsMeshTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsMesh3dTerrainTileEntity *entity = new QgsMesh3dTerrainTileEntity( terrain()->map3D(), mTriangularMesh, mSymbol.get(), mNode->tileId(), parent );
  entity->build();
  createTexture( entity );

  return entity;
}

//
// QgsMeshTerrainGenerator
//

QgsMeshTerrainGenerator::QgsMeshTerrainGenerator()
  : mSymbol( std::make_unique< QgsMesh3DSymbol >() )
{

}

QgsChunkLoader *QgsMeshTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  Q_ASSERT( meshLayer() );

  return new QgsMeshTerrainTileLoader( mTerrain, node, mTriangularMesh, symbol() );
}

float QgsMeshTerrainGenerator::rootChunkError( const Qgs3DMapSettings & ) const
{
  return 0;
}

void QgsMeshTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();

  for ( int i = 0; i < mTriangularMesh.vertices().count(); ++i )
  {
    float zValue = static_cast< float >( mTriangularMesh.vertices().at( i ).z() );
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
  setLayer( qobject_cast<QgsMeshLayer *>( project.mapLayer( mLayer.layerId ) ) );
}

void QgsMeshTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  if ( mLayer.get() )
    disconnect( mLayer.get(), &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::terrainChanged );

  mLayer = QgsMapLayerRef( layer );
  mIsValid = layer != nullptr;
  updateTriangularMesh();
  if ( mIsValid )
  {
    connect( mLayer.get(), &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::updateTriangularMesh );
    connect( mLayer.get(), &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::terrainChanged );
  }
}


QgsMeshLayer *QgsMeshTerrainGenerator::meshLayer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayer.layer.data() );
}

QgsTerrainGenerator *QgsMeshTerrainGenerator::clone() const
{
  QgsMeshTerrainGenerator *cloned = new QgsMeshTerrainGenerator();
  cloned->mLayer = mLayer;
  cloned->mTerrainTilingScheme = mTerrainTilingScheme;
  cloned->mCrs = mCrs;
  cloned->mSymbol.reset( mSymbol->clone() );
  cloned->mTransformContext = mTransformContext;
  cloned->mTriangularMesh = mTriangularMesh;
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

float QgsMeshTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings & ) const
{
  QgsPointXY point( x, y );
  int faceIndex = mTriangularMesh.faceIndexForPoint_v2( point );
  if ( faceIndex < 0 || faceIndex >= mTriangularMesh.triangles().count() )
    return std::numeric_limits<float>::quiet_NaN();

  const QgsMeshFace &face = mTriangularMesh.triangles().at( faceIndex );

  QgsPoint p1 = mTriangularMesh.vertices().at( face.at( 0 ) );
  QgsPoint p2 = mTriangularMesh.vertices().at( face.at( 1 ) );
  QgsPoint p3 = mTriangularMesh.vertices().at( face.at( 2 ) );

  return QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, p1.z(), p2.z(), p3.z(), point );
}

void QgsMeshTerrainGenerator::updateTriangularMesh()
{
  if ( meshLayer() )
  {
    QgsCoordinateTransform transform( mCrs, meshLayer()->crs(), mTransformContext );
    meshLayer()->updateTriangularMesh( transform );
    mTriangularMesh = *meshLayer()->triangularMeshByLodIndex( mSymbol->levelOfDetailIndex() );
    mTerrainTilingScheme = QgsTilingScheme( mTriangularMesh.extent(), mCrs );
  }
  else
    mTerrainTilingScheme = QgsTilingScheme();
}

QgsMesh3DSymbol *QgsMeshTerrainGenerator::symbol() const
{
  return mSymbol.get();
}

void QgsMeshTerrainGenerator::setSymbol( QgsMesh3DSymbol *symbol )
{
  mSymbol.reset( symbol );
  updateTriangularMesh();
}

void QgsMeshTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
}
