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

#include "qgs3dmapsettings.h"
#include "qgs3drendercontext.h"
#include "qgschunkloader.h"
#include "qgsmaterial3dhandler.h"
#include "qgsmesh3dentity_p.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsmeshlayerutils.h"
#include "qgsterrainentity.h"

#include "moc_qgsmeshterraingenerator.cpp"

QgsTerrainGenerator *QgsMeshTerrainGenerator::create()
{
  return new QgsMeshTerrainGenerator();
}

QgsMeshTerrainGenerator::QgsMeshTerrainGenerator()
  : mSymbol( std::make_unique<QgsMesh3DSymbol>() )
{}

QFuture<QgsChunkLoaderResult> QgsMeshTerrainGenerator::loadChunk( QgsChunkNode *node )
{
  return loadTextureResources( node ).then( this, [this, node]( TerrainTextureResources resources ) {
    return QgsChunkLoaderResult { [this, node, resources]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
      Qgs3DRenderContext context = Qgs3DRenderContext::fromMapSettings( mTerrain->mapSettings() );
      QgsMesh3DTerrainTileEntity *entity = new QgsMesh3DTerrainTileEntity( context, mTriangularMesh, mSymbol.get(), node->tileId(), parent );
      entity->build();
      createTexture( entity, QgsMaterialContext::fromRenderContext( context ), resources );

      return entity;
    } };
  } );
}

float QgsMeshTerrainGenerator::rootChunkError( const Qgs3DMapSettings & ) const
{
  return 0;
}

void QgsMeshTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  float min = std::numeric_limits<float>::max();
  float max = -std::numeric_limits<float>::max();

  for ( int i = 0; i < mTriangularMesh.vertices().count(); ++i )
  {
    const float zValue = static_cast<float>( mTriangularMesh.vertices().at( i ).z() );
    if ( min > zValue )
      min = zValue;
    if ( max < zValue )
      max = zValue;
  }

  hMin = min;
  hMax = max;
}

void QgsMeshTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  if ( mLayer )
    disconnect( mLayer, &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::terrainChanged );

  mLayer = layer;
  mIsValid = layer;
  updateTriangularMesh();
  if ( mIsValid )
  {
    connect( mLayer, &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::updateTriangularMesh );
    connect( mLayer, &QgsMeshLayer::request3DUpdate, this, &QgsMeshTerrainGenerator::terrainChanged );
  }
}


QgsMeshLayer *QgsMeshTerrainGenerator::meshLayer() const
{
  return mLayer;
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

QgsTerrainGenerator::Type QgsMeshTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Mesh;
}

QgsRectangle QgsMeshTerrainGenerator::rootChunkExtent() const
{
  return mTriangularMesh.extent();
}

float QgsMeshTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext & ) const
{
  return QgsMeshLayerUtils::interpolateZForPoint( mTriangularMesh, x, y );
}

void QgsMeshTerrainGenerator::updateTriangularMesh()
{
  if ( meshLayer() )
  {
    const QgsCoordinateTransform transform( meshLayer()->crs(), mCrs, mTransformContext );
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
