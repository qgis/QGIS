/***************************************************************************
    qgsgeonoderequest.h
    ---------------------
    begin                : Jul 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgsgeonoderequest.h"

#include <QEventLoop>
#include <QNetworkCacheMetaData>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDomDocument>

QgsGeoNodeRequest::QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mBaseUrl( baseUrl )
  , mForceRefresh( forceRefresh )
{

}

QgsGeoNodeRequest::~QgsGeoNodeRequest()
{
  abort();
}

void QgsGeoNodeRequest::abort()
{
  mIsAborted = true;
  if ( mGeoNodeReply )
  {
    mGeoNodeReply->deleteLater();
    mGeoNodeReply = nullptr;
  }
}

void QgsGeoNodeRequest::fetchLayers()
{
  request( QStringLiteral( "/api/layers/" ) );
  QObject *obj = new QObject( this );

  connect( this, &QgsGeoNodeRequest::requestFinished, obj, [obj, this ]
  {
    QList<QgsGeoNodeRequest::ServiceLayerDetail> layers;
    if ( mError.isEmpty() )
    {
      layers = parseLayers( this->lastResponse() );
    }
    emit layersFetched( layers );

    obj->deleteLater();
  } );
}

QList<QgsGeoNodeRequest::ServiceLayerDetail> QgsGeoNodeRequest::fetchLayersBlocking()
{
  QList<QgsGeoNodeRequest::ServiceLayerDetail> layers;

  QEventLoop loop;
  connect( this, &QgsGeoNodeRequest::requestFinished, &loop, &QEventLoop::quit );
  QObject *obj = new QObject( this );
  connect( this, &QgsGeoNodeRequest::layersFetched, obj, [&]( const QList<QgsGeoNodeRequest::ServiceLayerDetail> &fetched )
  {
    layers = fetched;
  } );
  fetchLayers();
  loop.exec( QEventLoop::ExcludeUserInputEvents );
  delete obj;
  return layers;
}

QgsGeoNodeStyle QgsGeoNodeRequest::fetchDefaultStyleBlocking( const QString &layerName )
{
  QgsGeoNodeStyle defaultStyle;
  bool success = requestBlocking( QStringLiteral( "/api/layers?name=" )  + layerName );
  if ( !success )
  {
    return defaultStyle;
  }

  const QJsonDocument jsonDocument = QJsonDocument::fromJson( this->lastResponse() );
  const QJsonObject jsonObject = jsonDocument.object();
  const QList<QVariant> layers = jsonObject.toVariantMap().value( QStringLiteral( "objects" ) ).toList();
  if ( layers.count() < 1 )
  {
    return defaultStyle;
  }
  QString defaultStyleUrl = layers.at( 0 ).toMap().value( QStringLiteral( "default_style" ) ).toString();

  defaultStyle = retrieveStyle( defaultStyleUrl );

  return defaultStyle;

}

QList<QgsGeoNodeStyle> QgsGeoNodeRequest::fetchStylesBlocking( const QString &layerName )
{
  QList<QgsGeoNodeStyle> geoNodeStyles;
  bool success = requestBlocking( QStringLiteral( "/api/styles?layer__name=" ) + layerName );
  if ( !success )
  {
    return geoNodeStyles;
  }

  const QJsonDocument jsonDocument = QJsonDocument::fromJson( this->lastResponse() );
  const QJsonObject jsobObject = jsonDocument.object();
  const QList<QVariant> styles = jsobObject.toVariantMap().value( QStringLiteral( "objects" ) ).toList();

  for ( const QVariant &style : styles )
  {
    const QVariantMap styleMap = style.toMap();
    QString styleUrl = styleMap.value( QStringLiteral( "resource_uri" ) ).toString();
    QgsGeoNodeStyle geoNodeStyle = retrieveStyle( styleUrl );
    if ( !geoNodeStyle.name.isEmpty() )
    {
      geoNodeStyles.append( geoNodeStyle );
    }
  }

  return geoNodeStyles;

}

QgsGeoNodeStyle QgsGeoNodeRequest::fetchStyleBlocking( const QString &styleId )
{
  QString endPoint = QStringLiteral( "/api/styles/" ) + styleId;

  return retrieveStyle( endPoint );
}

void QgsGeoNodeRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of request downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( msg, 3 );
  emit statusChanged( msg );
}

QString QgsGeoNodeRequest::protocol() const
{
  return mProtocol;
}

void QgsGeoNodeRequest::setProtocol( const QString &protocol )
{
  mProtocol = protocol;
}

void QgsGeoNodeRequest::replyFinished()
{
  QgsDebugMsg( "Reply finished" );
  if ( !mIsAborted && mGeoNodeReply )
  {
    if ( mGeoNodeReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsg( "reply OK" );
      QVariant redirect = mGeoNodeReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {

        emit statusChanged( QStringLiteral( "GeoNode request redirected." ) );

        const QUrl &toUrl = redirect.toUrl();
        if ( toUrl == mGeoNodeReply->url() )
        {
          mError = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
          QgsMessageLog::logMessage( mError, tr( "GeoNode" ) );
          mHttpGeoNodeResponse.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );

          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mGeoNodeReply->deleteLater();
          mGeoNodeReply = nullptr;

          QgsDebugMsgLevel( QString( "redirected getcapabilities: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ), 3 );
          mGeoNodeReply = QgsNetworkAccessManager::instance()->get( request );

          connect( mGeoNodeReply, &QNetworkReply::finished, this, &QgsGeoNodeRequest::replyFinished, Qt::DirectConnection );
          connect( mGeoNodeReply, &QNetworkReply::downloadProgress, this, &QgsGeoNodeRequest::replyProgress, Qt::DirectConnection );
          return;
        }
      }
      else
      {
        const QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

        if ( nam->cache() )
        {
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mGeoNodeReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          const QNetworkCacheMetaData::RawHeaderList cmdHeaders = cmd.rawHeaders();
          for ( const QNetworkCacheMetaData::RawHeader &h : cmdHeaders )
          {
            if ( h.first != QStringLiteral( "Cache-Control" ) )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsg( QString( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ) );
          if ( cmd.expirationDate().isNull() )
          {
            QgsSettings settings;
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( settings.value( QStringLiteral( "qgis/defaultCapabilitiesExpiry" ), "24", QgsSettings::Providers ).toInt() * 60 * 60 ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsg( "No cache for capabilities!" );
        }

        mHttpGeoNodeResponse = mGeoNodeReply->readAll();

        if ( mHttpGeoNodeResponse.isEmpty() )
        {
          mError = tr( "empty of capabilities: %1" ).arg( mGeoNodeReply->errorString() );
        }
      }
    }
    else
    {
      mError = tr( "Request failed: %1" ).arg( mGeoNodeReply->errorString() );
      QgsMessageLog::logMessage( mError, tr( "GeoNode" ) );
      mHttpGeoNodeResponse.clear();
    }
  }

  if ( mGeoNodeReply )
  {
    mGeoNodeReply->deleteLater();
    mGeoNodeReply = nullptr;
  }

  emit requestFinished();
}

QList<QgsGeoNodeRequest::ServiceLayerDetail> QgsGeoNodeRequest::parseLayers( const QByteArray &layerResponse )
{
  QList<QgsGeoNodeRequest::ServiceLayerDetail> layers;
  if ( layerResponse.isEmpty() )
  {
    return layers;
  }

  const QJsonDocument jsonDocument = QJsonDocument::fromJson( layerResponse );
  const QJsonObject jsonObject = jsonDocument.object();
  const QVariantMap jsonVariantMap = jsonObject.toVariantMap();
  const QVariantList layerList = jsonVariantMap.value( QStringLiteral( "objects" ) ).toList();
  qint16 majorVersion;
  qint16 minorVersion;
  if ( jsonVariantMap.contains( QStringLiteral( "geonode_version" ) ) )
  {
    const QStringList geonodeVersionSplit = jsonVariantMap.value( QStringLiteral( "geonode_version" ) ).toString().split( '.' );
    majorVersion = geonodeVersionSplit.at( 0 ).toInt();
    minorVersion = geonodeVersionSplit.at( 1 ).toInt();
  }
  else
  {
    majorVersion = 2;
    minorVersion = 6;
  }

  if ( majorVersion == 2 && minorVersion == 6 )
  {
    for ( const QVariant &layer : qgsAsConst( layerList ) )
    {
      QgsGeoNodeRequest::ServiceLayerDetail layerStruct;
      const QVariantMap layerMap = layer.toMap();
      // Find WMS and WFS. XYZ is not available
      // Trick to get layer's typename from distribution_url or detail_url
      QString layerTypeName = layerMap.value( QStringLiteral( "detail_url" ) ).toString().split( '/' ).last();
      if ( layerTypeName.isEmpty() )
      {
        layerTypeName = layerMap.value( QStringLiteral( "distribution_url" ) ).toString().split( '/' ).last();
      }
      // On this step, layerTypeName is in WORKSPACE%3ALAYERNAME or WORKSPACE:LAYERNAME format
      if ( layerTypeName.contains( QStringLiteral( "%3A" ) ) )
      {
        layerTypeName.replace( QStringLiteral( "%3A" ), QStringLiteral( ":" ) );
      }
      // On this step, layerTypeName is in WORKSPACE:LAYERNAME format
      const QStringList splitURL = layerTypeName.split( ':' );
      QString layerWorkspace = splitURL.at( 0 );
      QString layerName = splitURL.at( 1 );

      layerStruct.name = layerName;
      layerStruct.typeName = layerTypeName;
      layerStruct.uuid = layerMap.value( QStringLiteral( "uuid" ) ).toString();
      layerStruct.title = layerMap.value( QStringLiteral( "title" ) ).toString();

      // WMS url : BASE_URI/geoserver/WORKSPACE/wms
      layerStruct.wmsURL = mBaseUrl + QStringLiteral( "/geoserver/" ) + layerWorkspace + QStringLiteral( "/wms" );
      // WFS url : BASE_URI/geoserver/WORKSPACE/wfs
      layerStruct.wfsURL = mBaseUrl + QStringLiteral( "/geoserver/" ) + layerWorkspace + QStringLiteral( "/wfs" );
      // XYZ url : set to empty string
      layerStruct.xyzURL.clear();

      layers.append( layerStruct );
    }
  }
  // Geonode version 2.7 or newer
  else if ( ( majorVersion == 2 && minorVersion >= 7 ) || ( majorVersion >= 3 ) )
  {
    for ( const QVariant &layer : qgsAsConst( layerList ) )
    {
      QgsGeoNodeRequest::ServiceLayerDetail layerStruct;
      const QVariantMap layerMap = layer.toMap();
      // Find WMS, WFS, and XYZ link
      const QVariantList layerLinks = layerMap.value( QStringLiteral( "links" ) ).toList();
      for ( const QVariant &link : layerLinks )
      {
        const QVariantMap linkMap = link.toMap();
        if ( linkMap.contains( QStringLiteral( "link_type" ) ) )
        {
          if ( linkMap.value( QStringLiteral( "link_type" ) ) == QStringLiteral( "OGC:WMS" ) )
          {
            layerStruct.wmsURL = linkMap.value( QStringLiteral( "url" ) ).toString();
          }
          else if ( linkMap.value( QStringLiteral( "link_type" ) ) == QStringLiteral( "OGC:WFS" ) )
          {
            layerStruct.wfsURL = linkMap.value( QStringLiteral( "url" ) ).toString();
          }
          else if ( linkMap.value( QStringLiteral( "link_type" ) ) == QStringLiteral( "image" ) )
          {
            if ( linkMap.contains( QStringLiteral( "name" ) ) && linkMap.value( QStringLiteral( "name" ) ) == QStringLiteral( "Tiles" ) )
            {
              layerStruct.xyzURL = linkMap.value( QStringLiteral( "url" ) ).toString();
            }
          }
        }
      }
      if ( layerMap.value( QStringLiteral( "typename" ) ).toString().isEmpty() )
      {
        const QStringList splitURL = layerMap.value( QStringLiteral( "detail_url" ) ).toString().split( '/' );
        layerStruct.typeName = splitURL.at( splitURL.length() - 1 );
      }
      layerStruct.uuid = layerMap.value( QStringLiteral( "uuid" ) ).toString();
      layerStruct.name = layerMap.value( QStringLiteral( "name" ) ).toString();
      layerStruct.typeName = layerMap.value( QStringLiteral( "typename" ) ).toString();
      layerStruct.title = layerMap.value( QStringLiteral( "title" ) ).toString();
      layers.append( layerStruct );
    }
  }
  return layers;
}

QgsGeoNodeStyle QgsGeoNodeRequest::retrieveStyle( const QString &styleUrl )
{
  QgsGeoNodeStyle geoNodeStyle;

  bool success = requestBlocking( styleUrl );
  if ( !success )
  {
    return geoNodeStyle;
  }
  const QJsonDocument jsonDocument = QJsonDocument::fromJson( this->lastResponse() );
  const QJsonObject jsonObject = jsonDocument.object();

  const QVariantMap jsonMap = jsonObject.toVariantMap();
  geoNodeStyle.id = jsonMap.value( QStringLiteral( "id" ) ).toString();
  geoNodeStyle.name = jsonMap.value( QStringLiteral( "name" ) ).toString();
  geoNodeStyle.title = jsonMap.value( QStringLiteral( "title" ) ).toString();
  geoNodeStyle.styleUrl = jsonMap.value( QStringLiteral( "style_url" ) ).toString();

  success = requestBlocking( geoNodeStyle.styleUrl );
  if ( !success )
  {
    return geoNodeStyle;
  }

  success = geoNodeStyle.body.setContent( this->lastResponse() );
  if ( !success )
  {
    return geoNodeStyle;
  }

  return geoNodeStyle;
}

QStringList QgsGeoNodeRequest::fetchServiceUrlsBlocking( const QString &serviceType )
{
  QStringList urls;

  const QList<QgsGeoNodeRequest::ServiceLayerDetail> layers = fetchLayersBlocking();

  if ( layers.empty() )
  {
    return urls;
  }

  for ( const QgsGeoNodeRequest::ServiceLayerDetail &layer : layers )
  {
    QString url;
    if ( QString::compare( serviceType, QStringLiteral( "wms" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.wmsURL;
    }
    else if ( QString::compare( serviceType, QStringLiteral( "wfs" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.wfsURL;
    }
    else if ( QString::compare( serviceType, QStringLiteral( "xyz" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.xyzURL;
    }

    if ( url.isEmpty() )
      continue;

    if ( !url.contains( QLatin1String( "://" ) ) )
    {
      url.prepend( protocol() );
    }
    if ( !urls.contains( url ) )
    {
      urls.append( url );
    }
  }

  return urls;
}

QgsStringMap QgsGeoNodeRequest::fetchServiceUrlDataBlocking( const QString &serviceType )
{
  QgsStringMap urls;

  const QList<QgsGeoNodeRequest::ServiceLayerDetail> layers = fetchLayersBlocking();

  if ( layers.empty() )
  {
    return urls;
  }

  for ( const QgsGeoNodeRequest::ServiceLayerDetail &layer : layers )
  {
    QString url;

    if ( QString::compare( serviceType, QStringLiteral( "wms" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.wmsURL;
    }
    else if ( QString::compare( serviceType, QStringLiteral( "wfs" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.wfsURL;
    }
    else if ( QString::compare( serviceType, QStringLiteral( "xyz" ), Qt::CaseInsensitive ) == 0 )
    {
      url = layer.xyzURL;
    }

    if ( url.isEmpty() )
      continue;

    QString layerName = layer.name;
    if ( !url.contains( QLatin1String( "://" ) ) )
    {
      url.prepend( protocol() );
    }
    if ( !urls.contains( url ) )
    {
      urls.insert( layerName, url );
    }
  }

  return urls;
}

void QgsGeoNodeRequest::request( const QString &endPoint )
{
  abort();
  mIsAborted = false;
  // Handle case where the endpoint is full url
  QString url = endPoint.startsWith( mBaseUrl ) ? endPoint : mBaseUrl + endPoint;
  QgsDebugMsg( "Requesting to " + url );
  setProtocol( url.split( QStringLiteral( "://" ) ).at( 0 ) );
  QUrl layerUrl( url );
  layerUrl.setScheme( protocol() );

  mError.clear();

  mGeoNodeReply = requestUrl( url );
  connect( mGeoNodeReply, &QNetworkReply::finished, this, &QgsGeoNodeRequest::replyFinished, Qt::DirectConnection );
  connect( mGeoNodeReply, &QNetworkReply::downloadProgress, this, &QgsGeoNodeRequest::replyProgress, Qt::DirectConnection );
}

bool QgsGeoNodeRequest::requestBlocking( const QString &endPoint )
{
  request( endPoint );

  QEventLoop loop;
  connect( this, &QgsGeoNodeRequest::requestFinished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mError.isEmpty();
}

QNetworkReply *QgsGeoNodeRequest::requestUrl( const QString &url )
{
  QNetworkRequest request( url );
  // Add authentication check here

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  return QgsNetworkAccessManager::instance()->get( request );
}


