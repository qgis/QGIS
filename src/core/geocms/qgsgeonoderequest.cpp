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
#include "qgsgeonoderequest.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

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
  , mGeoNodeReply( nullptr )
  , mIsAborted( false )
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

bool QgsGeoNodeRequest::getLayers()
{
  abort();
  mIsAborted = false;
  QgsMessageLog::logMessage( mBaseUrl, tr( "GeoNode" ) );
  QString url = mBaseUrl + QStringLiteral( "/api/layers/" );
  QgsMessageLog::logMessage( url, tr( "GeoNode" ) );
  QString protocol = url.split( "://" )[0];
  QUrl layerUrl( url );
  layerUrl.setScheme( protocol );

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

void QgsGeoNodeRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of request downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  QgsMessageLog::logMessage( QStringLiteral( "Reply in progress" ), tr( "GeoNode" ) );
  QgsMessageLog::logMessage( msg, tr( "GeoNode" ) );
  emit statusChanged( msg );
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

        emit statusChanged( tr( "GeoNode request redirected." ) );

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
            QgsSettings s;
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( QStringLiteral( "qgis/defaultCapabilitiesExpiry" ), "24" ).toInt() * 60 * 60 ) );
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

QList<LayerStruct> QgsGeoNodeRequest::parseLayers( QByteArray layerResponse )
{
  QList<LayerStruct> layers;
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
    for ( int i = 0; i < layerList.count(); i++ )
    {
      LayerStruct layerStruct;
      QVariantMap layer = layerList[i].toMap();
      // Find WMS and WFS. XYZ is not available
      // Trick to get layer's typename from distribution_url or detail_url
      QString layerTypeName  = layer["detail_url"].toString().split( "/" ).last();
      if ( layerTypeName.length() == 0 )
      {
        layerTypeName = layer["distribution_url"].toString().split( "/" ).last();
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
      layerStruct.uuid = layer["uuid"].toString();
      layerStruct.title = layer["title"].toString();;

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
    for ( int i = 0; i < layerList.count(); i++ )
    {
      LayerStruct layerStruct;
      QVariantMap layer = layerList[i].toMap();
      // Find WMS, WFS, and XYZ link
      QVariantList layerLinks = layer["links"].toList();
      layerStruct.wmsURL = QStringLiteral( "" );
      layerStruct.wfsURL = QStringLiteral( "" );
      layerStruct.xyzURL = QStringLiteral( "" );
      for ( int j = 0; j < layerLinks.count(); j++ )
      {
        QVariantMap link = layerLinks[j].toMap();
        if ( link.contains( "link_type" ) )
        {
          if ( link["link_type"] == "OGC:WMS" )
          {
            layerStruct.wmsURL = link["url"].toString();
          }
          if ( link["link_type"] == "OGC:WFS" )
          {
            layerStruct.wfsURL = link["url"].toString();
          }
          if ( link["link_type"] == "image" )
          {
            if ( link.contains( "name" ) && link["name"] == "Tiles" )
            {
              layerStruct.xyzURL = link["url"].toString();
            }
          }
        }
      }
      if ( layer["typename"].toString().length() == 0 )
      {
        QStringList splitURL = layer["detail_url"].toString().split( "/" );
        layerStruct.typeName = splitURL[ splitURL.length() - 1];
      }
      layerStruct.uuid = layer["uuid"].toString();
      layerStruct.name = layer["name"].toString();
      layerStruct.typeName = layer["typename"].toString();
      layerStruct.title = layer["title"].toString();
      layers.append( layerStruct );
    }
  }
  return layers;
}


QStringList QgsGeoNodeRequest::serviceUrls( QString serviceType )
{
  QStringList urls;
  bool success = getLayers();

  if ( !success )
  {
    return urls;
  }

  QList<LayerStruct> layers = parseLayers( this->response() );

  for ( int i = 0; i < layers.count(); i++ )
  {
    QString url;
    if ( serviceType.toLower() == "wms" )
    {
      url = layers[i].wmsURL;
    }
    else if ( serviceType.toLower() == "wfs" )
    {
      url = layers[i].wfsURL;
    }
    else if ( serviceType.toLower() == "xyz" )
    {
      url = layers[i].xyzURL;
    }
    else
    {
      url = "";
    }

    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      url.prepend( "http://" );
    }
    if ( !urls.contains( url ) && url.length() > 0 )
    {
      urls.append( url );
    }
  }

  return urls;
}


QgsStringMap QgsGeoNodeRequest::serviceUrlData( QString serviceType )
{
  QgsStringMap urls;
  bool success = getLayers();

  if ( !success )
  {
    return urls;
  }
  QList<LayerStruct> layers = parseLayers( this->response() );

  for ( int i = 0; i < layers.count(); i++ )
  {
    QString url;

    if ( serviceType.toLower() == "wms" )
    {
      url = layers[i].wmsURL;
    }
    else if ( serviceType.toLower() == "wfs" )
    {
      url = layers[i].wfsURL;
    }
    else if ( serviceType.toLower() == "xyz" )
    {
      url = layers[i].xyzURL;
    }
    else
    {
      url = "";
    }

    QString layerName = layers[i].name;
    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      // Change this to https (?)
      url.prepend( "http://" );
    }
    if ( !urls.contains( url ) && url.length() > 0 )
    {
      urls.insert( layerName, url );
    }
  }

  return urls;
}
