/***************************************************************************
  qgsdemterraingenerator.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdemterraingenerator.h"

#include "qgsdemterraintileloader_p.h"

#include "qgsrasterlayer.h"
#include "qgscoordinatetransform.h"

QgsDemTerrainGenerator::~QgsDemTerrainGenerator()
{
  delete mHeightMapGenerator;
}

void QgsDemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = QgsMapLayerRef( layer );
  updateGenerator();
}

QgsRasterLayer *QgsDemTerrainGenerator::layer() const
{
  return qobject_cast<QgsRasterLayer *>( mLayer.layer.data() );
}

void QgsDemTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
  updateGenerator();
}

QgsTerrainGenerator *QgsDemTerrainGenerator::clone() const
{
  QgsDemTerrainGenerator *cloned = new QgsDemTerrainGenerator;
  cloned->setTerrain( mTerrain );
  cloned->mCrs = mCrs;
  cloned->mLayer = mLayer;
  cloned->mResolution = mResolution;
  cloned->mSkirtHeight = mSkirtHeight;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Dem;
}

QgsRectangle QgsDemTerrainGenerator::extent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map )
  if ( mHeightMapGenerator )
    return mHeightMapGenerator->heightAt( x, y );
  else
    return 0;
}

void QgsDemTerrainGenerator::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( QStringLiteral( "layer" ), mLayer.layerId );
  elem.setAttribute( QStringLiteral( "resolution" ), mResolution );
  elem.setAttribute( QStringLiteral( "skirt-height" ), mSkirtHeight );

  // crs is not read/written - it should be the same as destination crs of the map
}

void QgsDemTerrainGenerator::readXml( const QDomElement &elem )
{
  mLayer = QgsMapLayerRef( elem.attribute( QStringLiteral( "layer" ) ) );
  mResolution = elem.attribute( QStringLiteral( "resolution" ) ).toInt();
  mSkirtHeight = elem.attribute( QStringLiteral( "skirt-height" ) ).toFloat();

  // crs is not read/written - it should be the same as destination crs of the map
}

void QgsDemTerrainGenerator::resolveReferences( const QgsProject &project )
{
  mLayer = QgsMapLayerRef( project.mapLayer( mLayer.layerId ) );
  updateGenerator();
}

QgsChunkLoader *QgsDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  // A bit of a hack to make cloning terrain generator work properly
  return new QgsDemTerrainTileLoader( mTerrain, node, const_cast<QgsDemTerrainGenerator *>( this ) );
}

void QgsDemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem )
  {
    QgsRectangle te = dem->extent();
    QgsCoordinateTransform terrainToMapTransform( dem->crs(), mCrs, mTransformContext );
    terrainToMapTransform.setBallparkTransformsAreAppropriate( true );
    te = terrainToMapTransform.transformBoundingBox( te );

    mTerrainTilingScheme = QgsTilingScheme( te, mCrs );
    delete mHeightMapGenerator;
    mHeightMapGenerator = new QgsDemHeightMapGenerator( dem, mTerrainTilingScheme, mResolution, mTransformContext );
    mIsValid = true;
  }
  else
  {
    mTerrainTilingScheme = QgsTilingScheme();
    delete mHeightMapGenerator;
    mHeightMapGenerator = nullptr;
    mIsValid = false;
  }
}
