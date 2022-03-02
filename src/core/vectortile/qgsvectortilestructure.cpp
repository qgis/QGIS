/***************************************************************************
  qgsvectortilestructure.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilestructure.h"
#include "qgstiles.h"
#include "qgsvectortileutils.h"

QgsVectorTileStructure QgsVectorTileStructure::fromWebMercator()
{
  QgsVectorTileStructure res;
  res.setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  res.setZ0xMin( -20037508.3427892 );
  res.setZ0xMax( 20037508.3427892 );
  res.setZ0yMin( -20037508.3427892 );
  res.setZ0yMax( 20037508.3427892 );
  res.setZ0Dimension( 2 * 20037508.3427892 );
  res.setZ0Scale( 559082264.0287178 );
  return res;
}

QgsTileMatrix QgsVectorTileStructure::tileMatrix( int zoom ) const
{
  return QgsTileMatrix::fromCustomDef( zoom, mCrs, QgsPointXY( mZ0xMin, mZ0yMax ), mZ0Dimension );
}

double QgsVectorTileStructure::scaleToZoom( double scale ) const
{
  return QgsVectorTileUtils::scaleToZoom( scale, mZ0Scale );
}

int QgsVectorTileStructure::scaleToZoomLevel( double scale ) const
{
  return QgsVectorTileUtils::scaleToZoomLevel( scale, mMinZoom, mMaxZoom, mZ0Scale );
}
