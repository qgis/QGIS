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

QgsGeoNodeRequest::QgsGeoNodeRequest( bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mForceRefresh( forceRefresh )
{

}

QgsGeoNodeRequest::QgsGeoNodeRequest( const QString &baseUrl, /*const QgsWmsAuthorization &auth,*/ bool forceRefresh, QObject *parent )
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

QList<QgsServiceLayerDetail> QgsGeoNodeRequest::getLayers()
{
  QList<QgsServiceLayerDetail> layers;
  bool success = request( QStringLiteral( "/api/layers/" ) );
  if ( !success )
  {
    return layers;
  }
  return parseLayers( this->response() );
}

QgsGeoNodeStyle QgsGeoNodeRequest::getDefaultStyle( const QString &layerName )
{
  QgsGeoNodeStyle defaultStyle;
  bool success = request( QStringLiteral( "/api/layers?name=" )  + layerName );
  if ( !success )
  {
    return defaultStyle;
  }

  QJsonDocument jsonDocument = QJsonDocument::fromJson( this->response() );
  QJsonObject jsonObject = jsonDocument.object();
  QList<QVariant> layers = jsonObject.toVariantMap()["objects"].toList();
  if ( layers.count() < 1 )
  {
    return defaultStyle;
  }
  QString defaultStyleUrl = layers[0].toMap()["default_style"].toString();

  defaultStyle = retrieveStyle( defaultStyleUrl );

  return defaultStyle;

}

QList<QgsGeoNodeStyle> QgsGeoNodeRequest::getStyles( const QString &layerName )
{
  QList<QgsGeoNodeStyle> geoNodeStyles;
  bool success = request( QStringLiteral( "/api/styles?layer__name=" ) + layerName );
  if ( !success )
  {
    return geoNodeStyles;
  }

  QJsonDocument jsonDocument = QJsonDocument::fromJson( this->response() );
  QJsonObject jsobObject = jsonDocument.object();
  QList<QVariant> styles = jsobObject.toVariantMap()["objects"].toList();

  Q_FOREACH ( QVariant style, styles )
  {
    QVariantMap styleMap = style.toMap();
    QString styleUrl = styleMap["resource_uri"].toString();
    QgsGeoNodeStyle geoNodeStyle = retrieveStyle( styleUrl );
    if ( !geoNodeStyle.name.isEmpty() )
    {
      geoNodeStyles.append( geoNodeStyle );
    }
  }

  return geoNodeStyles;

}

QgsGeoNodeStyle QgsGeoNodeRequest::getStyle( const QString &styleID )
{
  QString endPoint = QStringLiteral( "/api/styles/" ) + styleID;

  return retrieveStyle( endPoint );

}

void QgsGeoNodeRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of request downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsMessageLog::logMessage( msg, tr( "GeoNode" ) );
  emit statusChanged( msg );
}

QString QgsGeoNodeRequest::getProtocol() const
{
  return mProtocol;
}

void QgsGeoNodeRequest::setProtocol( const QString &protocol )
{
  mProtocol = protocol;
}

void QgsGeoNodeRequest::replyFinished()
{
  QgsMessageLog::logMessage( QStringLiteral( "Reply finished" ), tr( "GeoNode" ) );
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
        mGeoNodeReply->request();
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

          QgsDebugMsg( QString( "redirected getcapabilities: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ) );
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
          Q_FOREACH ( const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders() )
          {
            if ( h.first != "Cache-Control" )
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

QList<QgsServiceLayerDetail> QgsGeoNodeRequest::parseLayers( const QByteArray &layerResponse )
{
  QList<QgsServiceLayerDetail> layers;
  if ( layerResponse.isEmpty() )
  {
    return layers;
  }

  QJsonDocument jsonDocument = QJsonDocument::fromJson( layerResponse );
  QJsonObject jsonObject = jsonDocument.object();
  QVariantMap jsonVariantMap = jsonObject.toVariantMap();
  QVariantList layerList = jsonVariantMap["objects"].toList();
  qint16 majorVersion;
  qint16 minorVersion;
  if ( jsonVariantMap.contains( QStringLiteral( "geonode_version" ) ) )
  {
    QStringList geonodeVersionSplit = jsonVariantMap["geonode_version"].toString().split( "." );
    majorVersion = geonodeVersionSplit[0].toInt();
    minorVersion = geonodeVersionSplit[1].toInt();
  }
  else
  {
    majorVersion = 2;
    minorVersion = 6;
  }

  if ( majorVersion == 2 && minorVersion == 6 )
  {
    Q_FOREACH ( QVariant layer, layerList )
    {
      QgsServiceLayerDetail layerStruct;
      QVariantMap layerMap = layer.toMap();
      // Find WMS and WFS. XYZ is not available
      // Trick to get layer's typename from distribution_url or detail_url
      QString layerTypeName  = layerMap["detail_url"].toString().split( "/" ).last();
      if ( layerTypeName.length() == 0 )
      {
        layerTypeName = layerMap["distribution_url"].toString().split( "/" ).last();
      }
      // On this step, layerTypeName is in WORKSPACE%3ALAYERNAME or WORKSPACE:LAYERNAME format
      if ( layerTypeName.contains( "%3A" ) )
      {
        layerTypeName.replace( "%3A", ":" );
      }
      // On this step, layerTypeName is in WORKSPACE:LAYERNAME format
      QStringList splitURL = layerTypeName.split( ":" );
      QString layerWorkspace = splitURL[0];
      QString layerName = splitURL[1];

      layerStruct.name = layerName;
      layerStruct.typeName = layerTypeName;
      layerStruct.uuid = layerMap["uuid"].toString();
      layerStruct.title = layerMap["title"].toString();;

      // WMS url : BASE_URI/geoserver/WORKSPACE/wms
      layerStruct.wmsURL = mBaseUrl + "/geoserver/" + layerWorkspace + "/wms";
      // WFS url : BASE_URI/geoserver/WORKSPACE/wfs
      layerStruct.wfsURL = mBaseUrl + "/geoserver/" + layerWorkspace + "/wfs";
      // XYZ url : set to empty string
      layerStruct.xyzURL = "";

      layers.append( layerStruct );
    }
  }
  // Geonode version 2.7 or newer
  else if ( ( majorVersion == 2 && minorVersion >= 7 ) || ( majorVersion >= 3 ) )
  {
    Q_FOREACH ( QVariant layer, layerList )
    {
      QgsServiceLayerDetail layerStruct;
      QVariantMap layerMap = layer.toMap();
      // Find WMS, WFS, and XYZ link
      QVariantList layerLinks = layerMap["links"].toList();
      layerStruct.wmsURL = QStringLiteral( "" );
      layerStruct.wfsURL = QStringLiteral( "" );
      layerStruct.xyzURL = QStringLiteral( "" );
      Q_FOREACH ( QVariant link, layerLinks )
      {
        QVariantMap linkMap = link.toMap();
        if ( linkMap.contains( "link_type" ) )
        {
          if ( linkMap["link_type"] == "OGC:WMS" )
          {
            layerStruct.wmsURL = linkMap["url"].toString();
          }
          if ( linkMap["link_type"] == "OGC:WFS" )
          {
            layerStruct.wfsURL = linkMap["url"].toString();
          }
          if ( linkMap["link_type"] == "image" )
          {
            if ( linkMap.contains( "name" ) && linkMap["name"] == "Tiles" )
            {
              layerStruct.xyzURL = linkMap["url"].toString();
            }
          }
        }
      }
      if ( layerMap["typename"].toString().length() == 0 )
      {
        QStringList splitURL = layerMap["detail_url"].toString().split( "/" );
        layerStruct.typeName = splitURL[ splitURL.length() - 1];
      }
      layerStruct.uuid = layerMap["uuid"].toString();
      layerStruct.name = layerMap["name"].toString();
      layerStruct.typeName = layerMap["typename"].toString();
      layerStruct.title = layerMap["title"].toString();
      layers.append( layerStruct );
    }
  }
  return layers;
}

QgsGeoNodeStyle QgsGeoNodeRequest::retrieveStyle( const QString &styleUrl )
{
  QgsGeoNodeStyle geoNodeStyle;

  bool success = request( styleUrl );
  if ( !success )
  {
    return geoNodeStyle;
  }
  QJsonDocument jsonDocument = QJsonDocument::fromJson( this->response() );
  QJsonObject jsonObject = jsonDocument.object();

  geoNodeStyle.id = jsonObject.toVariantMap()["id"].toString();
  geoNodeStyle.name = jsonObject.toVariantMap()["name"].toString();
  geoNodeStyle.title = jsonObject.toVariantMap()["title"].toString();
  geoNodeStyle.styleUrl = jsonObject.toVariantMap()["style_url"].toString();

  success = request( geoNodeStyle.styleUrl );
  if ( !success )
  {
    return geoNodeStyle;
  }

  success = geoNodeStyle.body.setContent( this->response() );
  if ( !success )
  {
    return geoNodeStyle;
  }

  return geoNodeStyle;
}

QStringList QgsGeoNodeRequest::serviceUrls( const QString &serviceType )
{
  QStringList urls;

  QList<QgsServiceLayerDetail> layers = getLayers();

  if ( layers.empty() )
  {
    return urls;
  }

  Q_FOREACH ( QgsServiceLayerDetail layer, layers )
  {
    QString url;
    if ( serviceType.toLower() == "wms" )
    {
      url = layer.wmsURL;
    }
    else if ( serviceType.toLower() == "wfs" )
    {
      url = layer.wfsURL;
    }
    else if ( serviceType.toLower() == "xyz" )
    {
      url = layer.xyzURL;
    }
    else
    {
      url = "";
    }

    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      url.prepend( getProtocol() );
    }
    if ( !urls.contains( url ) && url.length() > 0 )
    {
      urls.append( url );
    }
  }

  return urls;
}

QgsStringMap QgsGeoNodeRequest::serviceUrlData( const QString &serviceType )
{
  QgsStringMap urls;

  QList<QgsServiceLayerDetail> layers = getLayers();

  if ( layers.empty() )
  {
    return urls;
  }

  Q_FOREACH ( QgsServiceLayerDetail layer, layers )
  {
    QString url;

    if ( serviceType.toLower() == "wms" )
    {
      url = layer.wmsURL;
    }
    else if ( serviceType.toLower() == "wfs" )
    {
      url = layer.wfsURL;
    }
    else if ( serviceType.toLower() == "xyz" )
    {
      url = layer.xyzURL;
    }
    else
    {
      url = "";
    }

    QString layerName = layer.name;
    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      url.prepend( getProtocol() );
    }
    if ( !urls.contains( url ) && url.length() > 0 )
    {
      urls.insert( layerName, url );
    }
  }

  return urls;
}

bool QgsGeoNodeRequest::request( const QString &endPoint )
{
  abort();
  mIsAborted = false;
  // Handle case where the endpoint is full url
  QString url = endPoint.startsWith( mBaseUrl ) ? endPoint : mBaseUrl + endPoint;
  QgsMessageLog::logMessage( "Requesting to " + url, tr( "GeoNode" ) );
  setProtocol( url.split( "://" )[0] );
  QUrl layerUrl( url );
  layerUrl.setScheme( getProtocol() );

  mError.clear();

  QNetworkRequest request( url );
  // Add authentication check here

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mGeoNodeReply = QgsNetworkAccessManager::instance()->get( request );

  connect( mGeoNodeReply, &QNetworkReply::finished, this, &QgsGeoNodeRequest::replyFinished, Qt::DirectConnection );
  connect( mGeoNodeReply, &QNetworkReply::downloadProgress, this, &QgsGeoNodeRequest::replyProgress, Qt::DirectConnection );

  QEventLoop loop;
  connect( this, &QgsGeoNodeRequest::requestFinished, &loop, &QEventLoop::quit );

  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mError.isEmpty();
}
