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
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include <QIcon>

///@cond PRIVATE

QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY = QStringLiteral( "mbtilesvectortiles" );
QString QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION = QObject::tr( "MBTile Vector Tiles data provider" );

QgsMbTilesVectorTileDataProvider::QgsMbTilesVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  const QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  QgsMbTiles reader( sourcePath );
  if ( !reader.open() )
  {
    QgsDebugMsg( QStringLiteral( "failed to open MBTiles file: " ) + sourcePath );
    mIsValid = false;
    return;
  }

  const QString format = reader.metadataValue( QStringLiteral( "format" ) );
  if ( format != QLatin1String( "pbf" ) )
  {
    QgsDebugMsg( QStringLiteral( "Cannot open MBTiles for vector tiles. Format = " ) + format );
    mIsValid = false;
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "name: " ) + reader.metadataValue( QStringLiteral( "name" ) ), 2 );

  mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator();

  bool minZoomOk, maxZoomOk;
  const int minZoom = reader.metadataValue( QStringLiteral( "minzoom" ) ).toInt( &minZoomOk );
  const int maxZoom = reader.metadataValue( QStringLiteral( "maxzoom" ) ).toInt( &maxZoomOk );
  if ( minZoomOk )
    mMatrixSet.dropMatricesOutsideZoomRange( minZoom, 99 );
  if ( maxZoomOk )
    mMatrixSet.dropMatricesOutsideZoomRange( 0, maxZoom );
  QgsDebugMsgLevel( QStringLiteral( "zoom range: %1 - %2" ).arg( mMatrixSet.minimumZoom() ).arg( mMatrixSet.maximumZoom() ), 2 );

  QgsRectangle r = reader.extent();
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                             QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext() );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    mExtent = ct.transformBoundingBox( r );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Could not transform layer extent to layer CRS" ) );
  }

  mIsValid = true;
}

QString QgsMbTilesVectorTileDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY;
}

QString QgsMbTilesVectorTileDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION;
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

  return mIsValid;
}

QgsRectangle QgsMbTilesVectorTileDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsCoordinateReferenceSystem QgsMbTilesVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

const QgsVectorTileMatrixSet &QgsMbTilesVectorTileDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
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


//
// QgsMbTilesVectorTileDataProviderMetadata
//

QgsMbTilesVectorTileDataProviderMetadata::QgsMbTilesVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY,
                         QgsMbTilesVectorTileDataProvider::MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION )
{
}

QgsMbTilesVectorTileDataProvider *QgsMbTilesVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsMbTilesVectorTileDataProvider( uri, options, flags );
}

QIcon QgsMbTilesVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QgsProviderMetadata::ProviderCapabilities QgsMbTilesVectorTileDataProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QVariantMap QgsMbTilesVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  // TODO -- carefully thin out options which don't apply to mbtiles

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), dsUri.param( QStringLiteral( "type" ) ) );
  if ( dsUri.hasParam( QStringLiteral( "serviceType" ) ) )
    uriComponents.insert( QStringLiteral( "serviceType" ), dsUri.param( QStringLiteral( "serviceType" ) ) );

  if ( uriComponents[ QStringLiteral( "type" ) ] == QLatin1String( "mbtiles" ) ||
       ( uriComponents[ QStringLiteral( "type" ) ] == QLatin1String( "xyz" ) &&
         !dsUri.param( QStringLiteral( "url" ) ).startsWith( QLatin1String( "http" ) ) ) )
  {
    uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  else
  {
    uriComponents.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  }

  if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
    uriComponents.insert( QStringLiteral( "zmin" ), dsUri.param( QStringLiteral( "zmin" ) ) );
  if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
    uriComponents.insert( QStringLiteral( "zmax" ), dsUri.param( QStringLiteral( "zmax" ) ) );

  dsUri.httpHeaders().updateMap( uriComponents );

  if ( dsUri.hasParam( QStringLiteral( "styleUrl" ) ) )
    uriComponents.insert( QStringLiteral( "styleUrl" ), dsUri.param( QStringLiteral( "styleUrl" ) ) );

  const QString authcfg = dsUri.authConfigId();
  if ( !authcfg.isEmpty() )
    uriComponents.insert( QStringLiteral( "authcfg" ), authcfg );

  return uriComponents;
}

QString QgsMbTilesVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  // TODO -- carefully thin out options which don't apply to mbtiles

  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), parts.value( QStringLiteral( "type" ) ).toString() );
  if ( parts.contains( QStringLiteral( "serviceType" ) ) )
    dsUri.setParam( QStringLiteral( "serviceType" ), parts[ QStringLiteral( "serviceType" ) ].toString() );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );

  if ( parts.contains( QStringLiteral( "zmin" ) ) )
    dsUri.setParam( QStringLiteral( "zmin" ), parts[ QStringLiteral( "zmin" ) ].toString() );
  if ( parts.contains( QStringLiteral( "zmax" ) ) )
    dsUri.setParam( QStringLiteral( "zmax" ), parts[ QStringLiteral( "zmax" ) ].toString() );

  dsUri.httpHeaders().setFromMap( parts );

  if ( parts.contains( QStringLiteral( "styleUrl" ) ) )
    dsUri.setParam( QStringLiteral( "styleUrl" ), parts[ QStringLiteral( "styleUrl" ) ].toString() );

  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts[ QStringLiteral( "authcfg" ) ].toString() );

  return dsUri.encodedUri();
}

QString QgsMbTilesVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  sourcePath = context.pathResolver().writePath( sourcePath );
  dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
  dsUri.setParam( QStringLiteral( "url" ), sourcePath );
  return dsUri.encodedUri();
}

QString QgsMbTilesVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  sourcePath = context.pathResolver().readPath( sourcePath );
  dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
  dsUri.setParam( QStringLiteral( "url" ), sourcePath );
  return dsUri.encodedUri();
}

QList<Qgis::LayerType> QgsMbTilesVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


///@endcond


