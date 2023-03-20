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
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QIcon>

///@cond PRIVATE


QString QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY = QStringLiteral( "vtpkvectortiles" );
QString QgsVtpkVectorTileDataProvider::DATA_PROVIDER_DESCRIPTION = QObject::tr( "VTPK Vector Tiles data provider" );


QgsVtpkVectorTileDataProvider::QgsVtpkVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  const QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  QgsVtpkTiles reader( sourcePath );
  if ( !reader.open() )
  {
    QgsDebugMsg( QStringLiteral( "failed to open VTPK file: " ) + sourcePath );
    mIsValid = false;
    return;
  }

  const QVariantMap metadata = reader.metadata();
  const QString format = metadata.value( QStringLiteral( "tileInfo" ) ).toMap().value( QStringLiteral( "format" ) ).toString();
  if ( format != QLatin1String( "pbf" ) )
  {
    QgsDebugMsg( QStringLiteral( "Cannot open VTPK for vector tiles. Format = " ) + format );
    mIsValid = false;
    return;
  }

  mMatrixSet = reader.matrixSet();
  mCrs = mMatrixSet.crs();
  mExtent = reader.extent( transformContext() );
  mLayerMetadata = reader.layerMetadata();

  mIsValid = true;
}

QgsVectorTileDataProvider::ProviderCapabilities QgsVtpkVectorTileDataProvider::providerCapabilities() const
{
  return QgsVectorTileDataProvider::ProviderCapability::ReadLayerMetadata;
}

QString QgsVtpkVectorTileDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_KEY;
}

QString QgsVtpkVectorTileDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_DESCRIPTION;
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

  return mIsValid;
}

QgsCoordinateReferenceSystem QgsVtpkVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QgsRectangle QgsVtpkVectorTileDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsLayerMetadata QgsVtpkVectorTileDataProvider::layerMetadata() const
{
  return mLayerMetadata;
}

const QgsVectorTileMatrixSet &QgsVtpkVectorTileDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
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


//
// QgsVtpkVectorTileDataProviderMetadata
//

QgsVtpkVectorTileDataProviderMetadata::QgsVtpkVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsVtpkVectorTileDataProvider::DATA_PROVIDER_KEY, QgsVtpkVectorTileDataProvider::DATA_PROVIDER_DESCRIPTION )
{
}

QgsVtpkVectorTileDataProvider *QgsVtpkVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsVtpkVectorTileDataProvider( uri, options, flags );
}

QIcon QgsVtpkVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QgsProviderMetadata::ProviderCapabilities QgsVtpkVectorTileDataProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QVariantMap QgsVtpkVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  // TODO -- carefully thin out options which don't apply to vtpk

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

QString QgsVtpkVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  // TODO -- carefully thin out options which don't apply to vtpk

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

QString QgsVtpkVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  sourcePath = context.pathResolver().writePath( sourcePath );
  dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
  dsUri.setParam( QStringLiteral( "url" ), sourcePath );
  return dsUri.encodedUri();
}

QString QgsVtpkVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  sourcePath = context.pathResolver().readPath( sourcePath );
  dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
  dsUri.setParam( QStringLiteral( "url" ), sourcePath );
  return dsUri.encodedUri();
}

QList<Qgis::LayerType> QgsVtpkVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


///@endcond


