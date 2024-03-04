/***************************************************************************
  qgsxyzvectortiledataprovider.cpp
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

#include "qgsxyzvectortiledataprovider.h"
#include "qgsthreadingutils.h"
#include "qgstiles.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include <QIcon>
#include <QNetworkRequest>

///@cond PRIVATE

QString QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_KEY = QStringLiteral( "xyzvectortiles" );
QString QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_DESCRIPTION = QObject::tr( "XYZ Vector Tiles data provider" );

//
// QgsXyzVectorTileDataProviderBase
//

QgsXyzVectorTileDataProviderBase::QgsXyzVectorTileDataProviderBase( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsVectorTileDataProvider( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();
}

QgsXyzVectorTileDataProviderBase::QgsXyzVectorTileDataProviderBase( const QgsXyzVectorTileDataProviderBase &other )
  : QgsVectorTileDataProvider( other )
{
  mAuthCfg = other.mAuthCfg;
  mHeaders = other.mHeaders;
}

bool QgsXyzVectorTileDataProviderBase::supportsAsync() const
{
  return true;
}

QgsVectorTileRawData QgsXyzVectorTileDataProviderBase::readTile( const QgsTileMatrixSet &set, const QgsTileXYZ &id, QgsFeedback *feedback ) const
{
  return QgsVectorTileRawData( id, loadFromNetwork( id, set.tileMatrix( id.zoomLevel() ), sourcePath(), mAuthCfg, mHeaders, feedback ) );
}

QList<QgsVectorTileRawData> QgsXyzVectorTileDataProviderBase::readTiles( const QgsTileMatrixSet &set, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback, Qgis::RendererUsage usage ) const
{
  QList<QgsVectorTileRawData> rawTiles;
  rawTiles.reserve( tiles.size() );
  const QString source = sourcePath();
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const QByteArray rawData = loadFromNetwork( id, set.tileMatrix( id.zoomLevel() ), source, mAuthCfg, mHeaders, feedback, usage );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QNetworkRequest QgsXyzVectorTileDataProviderBase::tileRequest( const QgsTileMatrixSet &set, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const
{
  QString urlTemplate = sourcePath();

  if ( urlTemplate.contains( QLatin1String( "{usage}" ) ) )
  {
    switch ( usage )
    {
      case Qgis::RendererUsage::View:
        urlTemplate.replace( QLatin1String( "{usage}" ), QLatin1String( "view" ) );
        break;
      case Qgis::RendererUsage::Export:
        urlTemplate.replace( QLatin1String( "{usage}" ), QLatin1String( "export" ) );
        break;
      case Qgis::RendererUsage::Unknown:
        urlTemplate.replace( QLatin1String( "{usage}" ), QString() );
        break;
    }
  }

  const QString url = QgsVectorTileUtils::formatXYZUrlTemplate( urlTemplate, id, set.tileMatrix( id.zoomLevel() ) );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsXyzVectorTileDataProvider" ) );
  QgsSetRequestInitiatorId( request, id.toString() );

  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), id.column() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), id.row() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ), id.zoomLevel() );

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mHeaders.updateNetworkRequest( request );

  if ( !mAuthCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
  {
    QgsMessageLog::logMessage( tr( "network request update failed for authentication config" ), tr( "Network" ) );
  }

  return request;
}

QByteArray QgsXyzVectorTileDataProviderBase::loadFromNetwork( const QgsTileXYZ &id, const QgsTileMatrix &tileMatrix, const QString &requestUrl, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback, Qgis::RendererUsage usage )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id, tileMatrix );

  if ( url.contains( QLatin1String( "{usage}" ) ) )
  {
    switch ( usage )
    {
      case Qgis::RendererUsage::View:
        url.replace( QLatin1String( "{usage}" ), QLatin1String( "view" ) );
        break;
      case Qgis::RendererUsage::Export:
        url.replace( QLatin1String( "{usage}" ), QLatin1String( "export" ) );
        break;
      case Qgis::RendererUsage::Unknown:
        url.replace( QLatin1String( "{usage}" ), QString() );
        break;
    }
  }

  QNetworkRequest nr;
  nr.setUrl( QUrl( url ) );

  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authid );
  QgsDebugMsgLevel( QStringLiteral( "Blocking request: " ) + url, 2 );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false, feedback );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( QStringLiteral( "Request failed: " ) + url );
    return QByteArray();
  }
  QgsNetworkReplyContent reply = req.reply();
  QgsDebugMsgLevel( QStringLiteral( "Request successful, content size %1" ).arg( reply.content().size() ), 2 );
  return reply.content();
}


//
// QgsXyzVectorTileDataProviderMetadata
//


QgsXyzVectorTileDataProviderMetadata::QgsXyzVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_KEY, QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_DESCRIPTION )
{
}

QgsXyzVectorTileDataProvider *QgsXyzVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsXyzVectorTileDataProvider( uri, options, flags );
}

QIcon QgsXyzVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QgsProviderMetadata::ProviderCapabilities QgsXyzVectorTileDataProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QVariantMap QgsXyzVectorTileDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );

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

QString QgsXyzVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
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

QString QgsXyzVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  const QUrl sourceUrl( sourcePath );
  if ( sourceUrl.isLocalFile() )
  {
    // relative path will become "file:./x.txt"
    const QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( relSrcUrl ).toString() );
    return dsUri.encodedUri();
  }

  return uri;
}

QString QgsXyzVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );

  const QUrl sourceUrl( sourcePath );
  if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
  {
    const QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( absSrcUrl ).toString() );
    return dsUri.encodedUri();
  }

  return uri;
}

QList<Qgis::LayerType> QgsXyzVectorTileDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}


//
// QgsXyzVectorTileDataProvider
//

QgsXyzVectorTileDataProvider::QgsXyzVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsXyzVectorTileDataProviderBase( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( !QgsVectorTileUtils::checkXYZUrlTemplate( sourcePath ) )
  {
    QgsDebugError( QStringLiteral( "Invalid format of URL for XYZ source: " ) + sourcePath );
    mIsValid = false;
    return;
  }

  int zMin = 0;
  if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
    zMin = dsUri.param( QStringLiteral( "zmin" ) ).toInt();

  int zMax = 14;
  if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
    zMax = dsUri.param( QStringLiteral( "zmax" ) ).toInt();

  mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator( zMin, zMax );
  mExtent = QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 );

  mIsValid = true;
}

QgsXyzVectorTileDataProvider::QgsXyzVectorTileDataProvider( const QgsXyzVectorTileDataProvider &other )
  : QgsXyzVectorTileDataProviderBase( other )
{
  mIsValid = other.mIsValid;
  mExtent = other.mExtent;
  mMatrixSet = other.mMatrixSet;
}

Qgis::DataProviderFlags QgsXyzVectorTileDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

QString QgsXyzVectorTileDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return XYZ_DATA_PROVIDER_KEY;
}

QString QgsXyzVectorTileDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return XYZ_DATA_PROVIDER_DESCRIPTION;
}

QgsVectorTileDataProvider *QgsXyzVectorTileDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsXyzVectorTileDataProvider( *this );
}

bool QgsXyzVectorTileDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QgsRectangle QgsXyzVectorTileDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsCoordinateReferenceSystem QgsXyzVectorTileDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

const QgsVectorTileMatrixSet &QgsXyzVectorTileDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
}

QString QgsXyzVectorTileDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  return dsUri.param( QStringLiteral( "url" ) );
}

///@endcond



