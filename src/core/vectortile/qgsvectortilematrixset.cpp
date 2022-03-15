/***************************************************************************
  qgsvectortilematrixset.cpp
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

#include "qgsvectortilematrixset.h"
#include "qgstiles.h"
#include "qgsvectortileutils.h"
#include "qgsarcgisrestutils.h"
#include "qgslogger.h"

QgsVectorTileMatrixSet QgsVectorTileMatrixSet::fromWebMercator()
{
  QgsVectorTileMatrixSet res;
  res.addGoogleCrs84QuadTiles();
  return res;
}

bool QgsVectorTileMatrixSet::fromEsriJson( const QVariantMap &json )
{
  setScaleToTileZoomMethod( Qgis::ScaleToTileZoomLevelMethod::Esri );

  const QVariantMap tileInfo = json.value( QStringLiteral( "tileInfo" ) ).toMap();

  const QVariantMap origin = tileInfo.value( QStringLiteral( "origin" ) ).toMap();
  const double originX = origin.value( QStringLiteral( "x" ) ).toDouble();
  const double originY = origin.value( QStringLiteral( "y" ) ).toDouble();

  const int rows = tileInfo.value( QStringLiteral( "rows" ), QStringLiteral( "512" ) ).toInt();
  const int cols = tileInfo.value( QStringLiteral( "cols" ), QStringLiteral( "512" ) ).toInt();
  if ( rows != cols )
  {
    QgsDebugMsg( QStringLiteral( "row/col size mismatch: %1 vs %2 - tile misalignment may occur" ).arg( rows ).arg( cols ) );
  }

  const QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::convertSpatialReference( tileInfo.value( QStringLiteral( "spatialReference" ) ).toMap() );

  const QVariantList lodList = tileInfo.value( QStringLiteral( "lods" ) ).toList();
  bool foundLevel0 = false;
  double z0Dimension = 0;

  for ( const QVariant &lod : lodList )
  {
    const QVariantMap lodMap = lod.toMap();
    const int level = lodMap.value( QStringLiteral( "level" ) ).toInt();
    if ( level == 0 )
    {
      z0Dimension = lodMap.value( QStringLiteral( "resolution" ) ).toDouble() * rows;
      foundLevel0 = true;
      break;
    }
  }

  if ( !foundLevel0 )
    return false;

  for ( const QVariant &lod : lodList )
  {
    const QVariantMap lodMap = lod.toMap();
    const int level = lodMap.value( QStringLiteral( "level" ) ).toInt();

    // TODO -- we shouldn't be using z0Dimension here, but rather the actual dimension and properties of
    // this exact LOD
    QgsTileMatrix tm = QgsTileMatrix::fromCustomDef(
                         level,
                         crs,
                         QgsPointXY( originX, originY ),
                         z0Dimension );
    tm.setScale( lodMap.value( QStringLiteral( "scale" ) ).toDouble() );
    addMatrix( tm );
  }
  return true;
}
