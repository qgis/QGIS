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

QgsRectangle QgsOnlineTerrainGenerator::extent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsOnlineTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( map )
  if ( mHeightMapGenerator )
    return mHeightMapGenerator->heightAt( x, y );
  else
    return 0;
}

void QgsOnlineTerrainGenerator::writeXml( QDomElement &elem ) const
{
  const QgsRectangle r = mExtent;
  QDomElement elemExtent = elem.ownerDocument().createElement( QStringLiteral( "extent" ) );
  elemExtent.setAttribute( QStringLiteral( "xmin" ), QString::number( r.xMinimum() ) );
  elemExtent.setAttribute( QStringLiteral( "xmax" ), QString::number( r.xMaximum() ) );
  elemExtent.setAttribute( QStringLiteral( "ymin" ), QString::number( r.yMinimum() ) );
  elemExtent.setAttribute( QStringLiteral( "ymax" ), QString::number( r.yMaximum() ) );
  elem.appendChild( elemExtent );

  elem.setAttribute( QStringLiteral( "resolution" ), mResolution );
  elem.setAttribute( QStringLiteral( "skirt-height" ), mSkirtHeight );

  // crs is not read/written - it should be the same as destination crs of the map
}

void QgsOnlineTerrainGenerator::readXml( const QDomElement &elem )
{
  mResolution = elem.attribute( QStringLiteral( "resolution" ) ).toInt();
  mSkirtHeight = elem.attribute( QStringLiteral( "skirt-height" ) ).toFloat();

  const QDomElement elemExtent = elem.firstChildElement( QStringLiteral( "extent" ) );
  const double xmin = elemExtent.attribute( QStringLiteral( "xmin" ) ).toDouble();
  const double xmax = elemExtent.attribute( QStringLiteral( "xmax" ) ).toDouble();
  const double ymin = elemExtent.attribute( QStringLiteral( "ymin" ) ).toDouble();
  const double ymax = elemExtent.attribute( QStringLiteral( "ymax" ) ).toDouble();

  setExtent( QgsRectangle( xmin, ymin, xmax, ymax ) );

  // crs is not read/written - it should be the same as destination crs of the map
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

  emit extentChanged();
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
