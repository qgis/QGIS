/***************************************************************************
  qgsmbtilesvectortiledataprovider.cpp
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

#include "qgsmbtilesvectortiledataprovider.h"
#include "qgsthreadingutils.h"
#include "qgsmbtiles.h"
#include "qgstiles.h"
#include "qgsvectortileloader.h"
#include "qgsziputils.h"
#include "qgslogger.h"

///@cond PRIVATE

QgsMbTilesVectorTileDataProvider::QgsMbTilesVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{

}

QgsVectorTileDataProvider *QgsMbTilesVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ProviderOptions options;
  options.transformContext = transformContext();
  return new QgsMbTilesVectorTileDataProvider( dataSourceUri(), options, mReadFlags );
}

QString QgsMbTilesVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( QStringLiteral( "url" ) );
}

bool QgsMbTilesVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

QgsCoordinateReferenceSystem QgsMbTilesVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

QByteArray QgsMbTilesVectorTileDataProvider::readTile( const QgsTileMatrix &, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsMbTiles mbReader( dsUri.param( QStringLiteral( "url" ) ) );
  mbReader.open();
  return loadFromMBTiles( mbReader, id, feedback );
}

QList<QgsVectorTileRawData> QgsMbTilesVectorTileDataProvider::readTiles( const QgsTileMatrix &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsMbTiles mbReader( dsUri.param( QStringLiteral( "url" ) ) );
  mbReader.open();

  QList<QgsVectorTileRawData> rawTiles;
  rawTiles.reserve( tiles.size() );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QByteArray rawData = loadFromMBTiles( mbReader, id, feedback );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QByteArray QgsMbTilesVectorTileDataProvider::loadFromMBTiles( QgsMbTiles &mbTileReader, const QgsTileXYZ &id, QgsFeedback *feedback )
{
  // MBTiles uses TMS specs with Y starting at the bottom while XYZ uses Y starting at the top
  const int rowTMS = static_cast<int>( pow( 2, id.zoomLevel() ) - id.row() - 1 );
  QByteArray gzippedTileData = mbTileReader.tileData( id.zoomLevel(), id.column(), rowTMS );
  if ( gzippedTileData.isEmpty() )
  {
    return QByteArray();
  }

  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  QByteArray data;
  if ( !QgsZipUtils::decodeGzip( gzippedTileData, data ) )
  {
    QgsDebugMsg( QStringLiteral( "Failed to decompress tile " ) + id.toString() );
    return QByteArray();
  }

  QgsDebugMsgLevel( QStringLiteral( "Tile blob size %1 -> uncompressed size %2" ).arg( gzippedTileData.size() ).arg( data.size() ), 2 );
  return data;
}
///@endcond


