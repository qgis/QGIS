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
#include "qgsarcgisrestutils.h"

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

bool QgsVectorTileStructure::fromEsriJson( const QVariantMap &json )
{
  const QVariantMap tileInfo = json.value( QStringLiteral( "tileInfo" ) ).toMap();

  const QVariantMap origin = tileInfo.value( QStringLiteral( "origin" ) ).toMap();
  const double originX = origin.value( QStringLiteral( "x" ) ).toDouble();
  const double originY = origin.value( QStringLiteral( "y" ) ).toDouble();

  const QVariantList lodList = tileInfo.value( QStringLiteral( "lods" ) ).toList();
  bool foundLevel0 = false;
  double z0Dimension = 0;
  double z0Scale = 0;
  int minLOD = -1;
  int maxLOD = -1;
  for ( const QVariant &lod : lodList )
  {
    const QVariantMap lodMap = lod.toMap();
    const int level = lodMap.value( QStringLiteral( "level" ) ).toInt();
    minLOD = minLOD == -1 ? level : std::min( level, minLOD );
    maxLOD = maxLOD == -1 ? level : std::max( level, maxLOD );
    if ( level == 0 )
    {
      z0Dimension = lodMap.value( QStringLiteral( "resolution" ) ).toDouble() * 512;
      z0Scale = lodMap.value( QStringLiteral( "scale" ) ).toDouble();
      foundLevel0 = true;
    }
  }

  if ( !foundLevel0 )
    return false;

  mZ0xMin = originX;
  mZ0xMax = originX + z0Dimension;
  mZ0yMin = originY - z0Dimension;
  mZ0yMax = originY;
  mZ0Dimension = z0Dimension;
  mZ0Scale = z0Scale;
  mCrs = QgsArcGisRestUtils::convertSpatialReference( tileInfo.value( QStringLiteral( "spatialReference" ) ).toMap() );
  mMinZoom = minLOD;
  mMaxZoom = maxLOD;
  return true;
}
