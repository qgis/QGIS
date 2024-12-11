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
#include "moc_qgsdemterraingenerator.cpp"

#include "qgsdemterraintileloader_p.h"

#include "qgs3dutils.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatetransform.h"

QgsTerrainGenerator *QgsDemTerrainGenerator::create()
{
  return new QgsDemTerrainGenerator();
}

QgsDemTerrainGenerator::~QgsDemTerrainGenerator()
{
  delete mHeightMapGenerator;
}

void QgsDemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = layer;
  updateGenerator();
}

QgsRasterLayer *QgsDemTerrainGenerator::layer() const
{
  return mLayer;
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
  cloned->mExtent = mExtent;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Dem;
}

QgsRectangle QgsDemTerrainGenerator::rootChunkExtent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( mHeightMapGenerator )
    return mHeightMapGenerator->heightAt( x, y );
  else
    return 0;
}

QgsChunkLoader *QgsDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  // A bit of a hack to make cloning terrain generator work properly
  return new QgsDemTerrainTileLoader( mTerrain, node, const_cast<QgsDemTerrainGenerator *>( this ) );
}

void QgsDemTerrainGenerator::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
  updateGenerator();
}

void QgsDemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem && mCrs.isValid() )
  {
    QgsRectangle layerExtent = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), mCrs, mTransformContext );
    // no need to have an mExtent larger than the actual layer's extent
    const QgsRectangle intersectExtent = mExtent.intersect( layerExtent );

    mTerrainTilingScheme = QgsTilingScheme( intersectExtent, mCrs );
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
