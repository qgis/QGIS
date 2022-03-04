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

QgsVectorTileMatrixSet QgsVectorTileMatrixSet::fromWebMercator()
{
  QgsVectorTileMatrixSet res;
  res.addGoogleCrs84QuadTiles();
  res.setZ0xMinimum( -20037508.3427892 );
  res.setZ0xMaximum( 20037508.3427892 );
  res.setZ0yMinimum( -20037508.3427892 );
  res.setZ0yMaximum( 20037508.3427892 );
  return res;
}

bool QgsVectorTileMatrixSet::readXml( const QDomElement &element, QgsReadWriteContext &context )
{
  if ( !QgsTileMatrixSet::readXml( element, context ) )
    return false;

  mZ0xMin = element.attribute( QStringLiteral( "z0xMin" ) ).toDouble();
  mZ0xMax = element.attribute( QStringLiteral( "z0xMax" ) ).toDouble();
  mZ0yMin = element.attribute( QStringLiteral( "z0yMin" ) ).toDouble();
  mZ0yMax = element.attribute( QStringLiteral( "z0yMax" ) ).toDouble();
  return true;
}

QDomElement QgsVectorTileMatrixSet::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement res = QgsTileMatrixSet::writeXml( document, context );
  res.setAttribute( QStringLiteral( "z0xMin" ), qgsDoubleToString( mZ0xMin ) );
  res.setAttribute( QStringLiteral( "z0xMax" ), qgsDoubleToString( mZ0xMax ) );
  res.setAttribute( QStringLiteral( "z0yMin" ), qgsDoubleToString( mZ0yMin ) );
  res.setAttribute( QStringLiteral( "z0yMax" ), qgsDoubleToString( mZ0yMax ) );
  return res;
}

bool QgsVectorTileMatrixSet::fromEsriJson( const QVariantMap &json )
{
  const QVariantMap tileInfo = json.value( QStringLiteral( "tileInfo" ) ).toMap();

  const QVariantMap origin = tileInfo.value( QStringLiteral( "origin" ) ).toMap();
  const double originX = origin.value( QStringLiteral( "x" ) ).toDouble();
  const double originY = origin.value( QStringLiteral( "y" ) ).toDouble();

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
      z0Dimension = lodMap.value( QStringLiteral( "resolution" ) ).toDouble() * 512;
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

  mZ0xMin = originX;
  mZ0yMax = originY;
  mZ0xMax = originX + z0Dimension;
  mZ0yMin = originY - z0Dimension;
  return true;
}
