/***************************************************************************
  qgsonlineterraingenerator.cpp
  --------------------------------------
  Date                 : March 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsonlineterraingenerator.h"
#include "moc_qgsonlineterraingenerator.cpp"

#include "qgsdemterraintileloader_p.h"


QgsOnlineTerrainGenerator::QgsOnlineTerrainGenerator() = default;

QgsOnlineTerrainGenerator::~QgsOnlineTerrainGenerator() = default;

QgsChunkLoader *QgsOnlineTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsDemTerrainTileLoader( mTerrain, node, const_cast<QgsOnlineTerrainGenerator *>( this ) );
}

QgsTerrainGenerator *QgsOnlineTerrainGenerator::clone() const
{
  QgsOnlineTerrainGenerator *cloned = new QgsOnlineTerrainGenerator;
  cloned->setTerrain( mTerrain );
  cloned->mCrs = mCrs;
  cloned->mExtent = mExtent;
  cloned->mResolution = mResolution;
  cloned->mSkirtHeight = mSkirtHeight;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsOnlineTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Online;
}

QgsRectangle QgsOnlineTerrainGenerator::rootChunkExtent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsOnlineTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( mHeightMapGenerator )
    return mHeightMapGenerator->heightAt( x, y );
  else
    return 0;
}

QgsTerrainGenerator *QgsOnlineTerrainGenerator::create()
{
  return new QgsOnlineTerrainGenerator();
}

void QgsOnlineTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
  updateGenerator();
}

void QgsOnlineTerrainGenerator::setExtent( const QgsRectangle &extent )
{
  if ( mExtent == extent )
    return;

  mExtent = extent;
  updateGenerator();
}

void QgsOnlineTerrainGenerator::updateGenerator()
{
  if ( mExtent.isNull() )
  {
    mTerrainTilingScheme = QgsTilingScheme();
  }
  else
  {
    // the real extent will be a square where the given extent fully fits
    mTerrainTilingScheme = QgsTilingScheme( mExtent, mCrs );
  }

  mHeightMapGenerator.reset( new QgsDemHeightMapGenerator( nullptr, mTerrainTilingScheme, mResolution, mTransformContext ) );
}
