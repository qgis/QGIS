/***************************************************************************
  qgsvtpkvectortiledataprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvtpkvectortiledataprovider.h"
#include "qgsthreadingutils.h"
#include "qgsvtpktiles.h"
#include "qgsvectortileloader.h"

///@cond PRIVATE

QgsVtpkVectorTileDataProvider::QgsVtpkVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{

}

QgsVectorTileDataProvider *QgsVtpkVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ProviderOptions options;
  options.transformContext = transformContext();
  return new QgsVtpkVectorTileDataProvider( dataSourceUri(), options, mReadFlags );
}

QString QgsVtpkVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( QStringLiteral( "url" ) );
}

bool QgsVtpkVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

QgsCoordinateReferenceSystem QgsVtpkVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

QByteArray QgsVtpkVectorTileDataProvider::readTile( const QgsTileMatrix &, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  QgsVtpkTiles reader( dsUri.param( QStringLiteral( "url" ) ) );
  reader.open();
  return loadFromVtpk( reader, id, feedback );
}

QList<QgsVectorTileRawData> QgsVtpkVectorTileDataProvider::readTiles( const QgsTileMatrix &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsVtpkTiles reader( dsUri.param( QStringLiteral( "url" ) ) );
  reader.open();

  QList<QgsVectorTileRawData> rawTiles;
  rawTiles.reserve( tiles.size() );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QByteArray rawData = loadFromVtpk( reader, id, feedback );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QByteArray QgsVtpkVectorTileDataProvider::loadFromVtpk( QgsVtpkTiles &vtpkTileReader, const QgsTileXYZ &id, QgsFeedback * )
{
  const QByteArray tileData = vtpkTileReader.tileData( id.zoomLevel(), id.column(), id.row() );
  if ( tileData.isEmpty() )
  {
    return QByteArray();
  }
  return tileData;
}

///@endcond


