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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsthreadingutils.h"
#include "qgstiles.h"
#include "qgsvectortiledataprovider.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"

#include <QIcon>
#include <QNetworkRequest>

#include "moc_qgsxyzvectortiledataprovider.cpp"

///@cond PRIVATE

QString QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_KEY = u"xyzvectortiles"_s;
QString QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_DESCRIPTION = QObject::tr( "XYZ Vector Tiles data provider" );

//
// QgsXyzVectorTileDataProviderBase
//

QgsXyzVectorTileDataProviderBase::QgsXyzVectorTileDataProviderBase( const QString &uri, const ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
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
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    QMap<QString, QByteArray> data;
    const QgsStringMap sources = sourcePaths();
    QgsStringMap::const_iterator it = sources.constBegin();
    for ( ; it != sources.constEnd(); ++it )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      const QByteArray rawData = loadFromNetwork( id, set.tileMatrix( id.zoomLevel() ), it.value(), mAuthCfg, mHeaders, feedback, usage );
      if ( !rawData.isEmpty() )
      {
        data[it.key()] = rawData;
      }
    }
    rawTiles.append( QgsVectorTileRawData( id, data ) );
  }
  return rawTiles;
}

QList<QNetworkRequest> QgsXyzVectorTileDataProviderBase::tileRequests( const QgsTileMatrixSet &set, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const
{
  QList<QNetworkRequest> requests;

  const QgsStringMap sourcesPaths = sourcePaths();

  QgsStringMap::const_iterator it = sourcesPaths.constBegin();

  for ( ; it != sourcesPaths.constEnd(); ++it )
  {
    QString urlTemplate = it.value();
    QString layerName = it.key();

    if ( urlTemplate.contains( "{usage}"_L1 ) )
    {
      switch ( usage )
      {
        case Qgis::RendererUsage::View:
          urlTemplate.replace( "{usage}"_L1, "view"_L1 );
          break;
        case Qgis::RendererUsage::Export:
          urlTemplate.replace( "{usage}"_L1, "export"_L1 );
          break;
        case Qgis::RendererUsage::Unknown:
          urlTemplate.replace( "{usage}"_L1, QString() );
          break;
      }
    }

    const QString url = QgsVectorTileUtils::formatXYZUrlTemplate( urlTemplate, id, set.tileMatrix( id.zoomLevel() ) );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, u"QgsXyzVectorTileDataProvider"_s );
    QgsSetRequestInitiatorId( request, id.toString() );

    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsVectorTileDataProvider::DATA_COLUMN ), id.column() );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsVectorTileDataProvider::DATA_ROW ), id.row() );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsVectorTileDataProvider::DATA_ZOOM ), id.zoomLevel() );
    request.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsVectorTileDataProvider::DATA_SOURCE_ID ), layerName );

    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    mHeaders.updateNetworkRequest( request );

    if ( !mAuthCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
    {
      QgsMessageLog::logMessage( tr( "network request update failed for authentication config" ), tr( "Network" ) );
    }

    requests << request;
  }

  return requests;
}

QByteArray QgsXyzVectorTileDataProviderBase::loadFromNetwork( const QgsTileXYZ &id, const QgsTileMatrix &tileMatrix, const QString &requestUrl, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback, Qgis::RendererUsage usage )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id, tileMatrix );

  if ( url.contains( "{usage}"_L1 ) )
  {
    switch ( usage )
    {
      case Qgis::RendererUsage::View:
        url.replace( "{usage}"_L1, "view"_L1 );
        break;
      case Qgis::RendererUsage::Export:
        url.replace( "{usage}"_L1, "export"_L1 );
        break;
      case Qgis::RendererUsage::Unknown:
        url.replace( "{usage}"_L1, QString() );
        break;
    }
  }

  QNetworkRequest nr;
  nr.setUrl( QUrl( url ) );

  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authid );
  QgsDebugMsgLevel( u"Blocking request: "_s + url, 2 );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false, feedback );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( u"Request failed: "_s + url );
    return QByteArray();
  }
  QgsNetworkReplyContent reply = req.reply();
  QgsDebugMsgLevel( u"Request successful, content size %1"_s.arg( reply.content().size() ), 2 );
  return reply.content();
}


//
// QgsXyzVectorTileDataProviderMetadata
//


QgsXyzVectorTileDataProviderMetadata::QgsXyzVectorTileDataProviderMetadata()
  : QgsProviderMetadata( QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_KEY, QgsXyzVectorTileDataProvider::XYZ_DATA_PROVIDER_DESCRIPTION )
{
}

QgsXyzVectorTileDataProvider *QgsXyzVectorTileDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsXyzVectorTileDataProvider( uri, options, flags );
}

QIcon QgsXyzVectorTileDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconVectorTileLayer.svg"_s );
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
  uriComponents.insert( u"type"_s, u"xyz"_s );

  if ( uriComponents[ u"type"_s ] == "mbtiles"_L1 ||
       ( uriComponents[ u"type"_s ] == "xyz"_L1 &&
         !dsUri.param( u"url"_s ).startsWith( "http"_L1 ) ) )
  {
    uriComponents.insert( u"path"_s, dsUri.param( u"url"_s ) );
  }
  else
  {
    uriComponents.insert( u"url"_s, dsUri.param( u"url"_s ) );
    if ( dsUri.hasParam( u"urlName"_s ) )
      uriComponents.insert( u"urlName"_s, dsUri.param( u"urlName"_s ) );
  }
  int i = 2;
  while ( true )
  {
    QString url = dsUri.param( u"url_%2"_s.arg( i ) );
    QString urlName = dsUri.param( u"urlName_%2"_s.arg( i ) );
    if ( url.isEmpty() || urlName.isEmpty() )
      break;
    uriComponents.insert( u"urlName_%2"_s.arg( i ), urlName );
    uriComponents.insert( u"url_%2"_s.arg( i ), url );
    i++;
  }


  if ( dsUri.hasParam( u"zmin"_s ) )
    uriComponents.insert( u"zmin"_s, dsUri.param( u"zmin"_s ) );
  if ( dsUri.hasParam( u"zmax"_s ) )
    uriComponents.insert( u"zmax"_s, dsUri.param( u"zmax"_s ) );

  dsUri.httpHeaders().updateMap( uriComponents );

  if ( dsUri.hasParam( u"styleUrl"_s ) )
    uriComponents.insert( u"styleUrl"_s, dsUri.param( u"styleUrl"_s ) );

  const QString authcfg = dsUri.authConfigId();
  if ( !authcfg.isEmpty() )
    uriComponents.insert( u"authcfg"_s, authcfg );

  return uriComponents;
}

QString QgsXyzVectorTileDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, u"xyz"_s );
  dsUri.setParam( u"url"_s, parts.value( parts.contains( u"path"_s ) ? u"path"_s : u"url"_s ).toString() );
  if ( parts.contains( u"urlName"_s ) )
    dsUri.setParam( u"urlName"_s, parts[ u"urlName"_s ].toString() );

  int i = 2;
  while ( true )
  {
    QString urlNameKey = u"urlName_%2"_s.arg( i );
    QString urlKey = u"url_%2"_s.arg( i );

    if ( !parts.contains( urlNameKey ) || !parts.contains( urlKey ) )
      break;
    QString url = dsUri.param( u"url_%2"_s.arg( i ) );
    QString urlName = dsUri.param( u"urlName_%2"_s.arg( i ) );
    if ( url.isEmpty() || urlName.isEmpty() )
      break;

    dsUri.setParam( urlNameKey, parts[ urlNameKey ].toString() );
    dsUri.setParam( urlKey, parts[ urlKey ].toString() );
    i++;
  }

  if ( parts.contains( u"zmin"_s ) )
    dsUri.setParam( u"zmin"_s, parts[ u"zmin"_s ].toString() );
  if ( parts.contains( u"zmax"_s ) )
    dsUri.setParam( u"zmax"_s, parts[ u"zmax"_s ].toString() );

  dsUri.httpHeaders().setFromMap( parts );

  if ( parts.contains( u"styleUrl"_s ) )
    dsUri.setParam( u"styleUrl"_s, parts[ u"styleUrl"_s ].toString() );

  if ( parts.contains( u"authcfg"_s ) )
    dsUri.setAuthConfigId( parts[ u"authcfg"_s ].toString() );

  return dsUri.encodedUri();
}

QString QgsXyzVectorTileDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( u"url"_s );

  const QUrl sourceUrl( sourcePath );
  if ( sourceUrl.isLocalFile() )
  {
    // relative path will become "file:./x.txt"
    const QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
    dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
    dsUri.setParam( u"url"_s, QUrl::fromLocalFile( relSrcUrl ).toString( QUrl::DecodeReserved ) );
    return dsUri.encodedUri();
  }

  return uri;
}

QString QgsXyzVectorTileDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QString sourcePath = dsUri.param( u"url"_s );

  const QUrl sourceUrl( sourcePath );
  if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
  {
    const QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
    dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
    dsUri.setParam( u"url"_s, QUrl::fromLocalFile( absSrcUrl ).toString( QUrl::DecodeReserved ) );
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

QgsXyzVectorTileDataProvider::QgsXyzVectorTileDataProvider( const QString &uri, const ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
  : QgsXyzVectorTileDataProviderBase( uri, providerOptions, flags )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourcePath = dsUri.param( u"url"_s );
  if ( !QgsVectorTileUtils::checkXYZUrlTemplate( sourcePath ) )
  {
    QgsDebugError( u"Invalid format of URL for XYZ source: "_s + sourcePath );
    mIsValid = false;
    return;
  }

  int zMin = 0;
  if ( dsUri.hasParam( u"zmin"_s ) )
    zMin = dsUri.param( u"zmin"_s ).toInt();

  int zMax = 14;
  if ( dsUri.hasParam( u"zmax"_s ) )
    zMax = dsUri.param( u"zmax"_s ).toInt();

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

  return QgsCoordinateReferenceSystem( u"EPSG:3857"_s );
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
  return dsUri.param( u"url"_s );
}

QgsStringMap QgsXyzVectorTileDataProvider::sourcePaths() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  QgsStringMap paths = { { dsUri.param( u"urlName"_s ), dsUri.param( u"url"_s ) } };

  int i = 2;
  while ( true )
  {
    QString url = dsUri.param( u"url_%2"_s.arg( i ) );
    QString urlName = dsUri.param( u"urlName_%2"_s.arg( i ) );
    if ( url.isEmpty() || urlName.isEmpty() )
      break;

    paths.insert( urlName, url );
    i++;
  }

  return paths;
}

///@endcond



