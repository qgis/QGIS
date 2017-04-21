/***************************************************************************
    qgsgeonodeconnection.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeonodeconnection.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"

#include <QMultiMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonDocument>
#include <QDebug>
#include <QUrl>
#include <QDomDocument>


const QString QgsGeoNodeConnection::pathGeoNodeConnection = "qgis/connections-geonode";
const QString QgsGeoNodeConnection::pathGeoNodeConnectionDetails = "qgis/GeoNode";

QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &connName )
  : mConnName( connName )
{
  QgsDebugMsg( "theConnName = " + connName );

  QgsSettings settings;

  QString key = pathGeoNodeConnection + "/" + mConnName;
  QString credentialsKey = pathGeoNodeConnectionDetails + "/" + mConnName;

  QStringList connStringParts;

  mUri.setParam( QStringLiteral( "url" ), settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }

  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsGeoNodeConnection::~QgsGeoNodeConnection()
{

}

QgsDataSourceUri QgsGeoNodeConnection::uri()
{
  return mUri;
}

QStringList QgsGeoNodeConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( pathGeoNodeConnection );
  return settings.childGroups();
}

void QgsGeoNodeConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( pathGeoNodeConnection + '/' + name );
  settings.remove( pathGeoNodeConnectionDetails + '/' + name );
}

QString QgsGeoNodeConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( pathGeoNodeConnection + QStringLiteral( "/selected" ) ).toString();
}

void QgsGeoNodeConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( pathGeoNodeConnection + QStringLiteral( "/selected" ), name );
}

QVariantList QgsGeoNodeConnection::getLayers()
{
  // Construct URL. I need to prepend http in the beginning to make it work.
  // setScheme doesn't really help.
  QString url = uri().param( "url" ) + QStringLiteral( "/api/layers/" );
  if ( !url.contains( QLatin1String( "://" ) ) )
  {
    url.prepend( "http://" );
  }
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  // Handle redirect
  // request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );

  QNetworkReply *reply = networkManager->get( request );
  while ( !reply->isFinished() )
  {
    qApp->processEvents();
  }
  QByteArray response_data = reply->readAll();
  QJsonDocument jsonDocument = QJsonDocument::fromJson( response_data );
  QJsonObject jsonObject = jsonDocument.object();
  QVariantMap jsonVariantMap = jsonObject.toVariantMap();
  QVariantList layerList = jsonVariantMap["objects"].toList();
  for ( int i = 0; i < layerList.count(); ++i )
  {
    QVariantMap layer = layerList[i].toMap();

    // Trick to get layer's name from distribution_url or detail_url
    QStringList temp = layer["distribution_url"].toString().split( "/" );
    QString layerName = temp[temp.count() - 1];
    if ( layerName.length() == 0 )
    {
      temp = layer["detail_url"].toString().split( "/" );
      layerName = temp[temp.count() - 1];
    }
    // Add typeName
    QString layerTypeName = layerName;
    layer[QStringLiteral( "typename" )] = layerName;
    // Clean from geonode%3A
    QString geonodePrefix = QStringLiteral( "geonode%3A" );
    if ( layerName.contains( geonodePrefix ) )
    {
      layerName.remove( 0, geonodePrefix.length() );
    }
    layer[QStringLiteral( "name" )] = layerName;
    layerList[i] = layer;
  }
  return layerList;
}

QVariantList QgsGeoNodeConnection::getLayers( QString serviceType )
{
  QString param = QString( "?version=2.0.0&service=%1&request=GetCapabilities" ).arg( serviceType.toLower() );
  QString url = serviceUrl( serviceType ) + param;

  if ( !url.contains( QLatin1String( "://" ) ) )
  {
    url.prepend( "http://" );
  }
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/xml" );

  QNetworkReply *reply = networkManager->get( request );
  while ( !reply->isFinished() )
  {
    qApp->processEvents();
  }
  QByteArray response_data = reply->readAll();

  QDomDocument dom;
  QDomNodeList featureNodeList;
  dom.setContent( response_data );

  if ( serviceType == QString( "WFS" ) )
  {
    featureNodeList = dom.elementsByTagName( "FeatureType" );
  }
  else
  {
    featureNodeList = dom.elementsByTagName( "Layer" );
  }

  QVariantList layerList;

  if ( !featureNodeList.isEmpty() )
  {
    for ( int i = serviceType == QString( "WMS" ) ? 1 : 0; i < featureNodeList.size(); ++i )
    {
      QVariantMap layer;

      QDomNode featureNode = featureNodeList.at( i );
      QString layerName = featureNode.namedItem( "Name" ).firstChild().nodeValue();
      QString layerTitle = featureNode.namedItem( "Title" ).firstChild().nodeValue();
      QString layerCRS;

      if ( serviceType == QString( "WFS" ) )
      {
        layerCRS = featureNode.namedItem( "DefaultCRS" ).firstChild().nodeValue();
      }
      else
      {
        layerCRS = featureNode.namedItem( "CRS" ).firstChild().nodeValue();
      }

      QString geonodePrefix = QStringLiteral( "geonode:" );
      QString CRSPrefix = QStringLiteral( "urn:ogc:def:crs:" );

      layer[QStringLiteral( "typename" )] = layerName;

      if ( layerName.contains( geonodePrefix ) )
      {
        layerName.remove( 0, geonodePrefix.length() );
      }
      if ( layerCRS.contains( CRSPrefix ) )
      {
        layerCRS.remove( 0, CRSPrefix.length() );
      }

      layer.insertMulti( QStringLiteral( "name" ), layerName );
      layer.insertMulti( QStringLiteral( "title" ), layerTitle );
      layer.insertMulti( QStringLiteral( "default_crs" ), layerCRS );
      layerList.append( layer );
    }
  }

  return layerList;
}

// Currently copy and paste from getLayers. It can be refactored easily, difference in url only.
QVariantList QgsGeoNodeConnection::getMaps()
{
  // Construct URL. I need to prepend http in the beginning to make it work.
  // setScheme doesn't really help.
  QString url = uri().param( "url" ) + QStringLiteral( "/api/maps/" );
  if ( !url.contains( QLatin1String( "://" ) ) )
  {
    url.prepend( "http://" );
  }
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  // Handle redirect
  // request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );

  QNetworkReply *reply = networkManager->get( request );
  while ( !reply->isFinished() )
  {
    qApp->processEvents();
  }
  QByteArray response_data = reply->readAll();
  QJsonDocument jsonDocument = QJsonDocument::fromJson( response_data );
  QJsonObject jsonObject = jsonDocument.object();
  QVariantMap jsonVariantMap = jsonObject.toVariantMap();
  QVariantList layerList = jsonVariantMap["objects"].toList();

  return layerList;
}

QString QgsGeoNodeConnection::serviceUrl( QString &resourceID, QString serviceType )
{
  // Example CSW url
  // demo.geonode.org/catalogue/csw?request=GetRecordById&service=CSW&version=2.0.2&elementSetName=full&id=

  QString url = uri().param( "url" ) + QString( "/catalogue/csw?request=GetRecordById&service=CSW&version=2.0.2&elementSetName=full&id=%1" ).arg( resourceID );
  if ( !url.contains( QLatin1String( "://" ) ) )
  {
    url.prepend( "http://" );
  }
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/xml" );

  QNetworkReply *reply = networkManager->get( request );
  while ( !reply->isFinished() )
  {
    qApp->processEvents();
  }
  QByteArray response_data = reply->readAll();

  QDomDocument dom;
  dom.setContent( response_data );

  QDomNodeList referenceNodeList = dom.elementsByTagName( "dct:references" );

  if ( !referenceNodeList.isEmpty() )
  {
    for ( int i = 0; i < referenceNodeList.size(); ++i )
    {
      QDomNode referenceNode = referenceNodeList.at( i );
      QDomNamedNodeMap attributes = referenceNode.attributes();
      QString scheme = attributes.namedItem( "scheme" ).firstChild().nodeValue();
      // Trick to get the WMS / WFS Url from CSW
      if ( serviceType == QStringLiteral( "WMS" ) || serviceType == QStringLiteral( "WFS" ) )
      {
        if ( scheme.startsWith( "OGC" ) && scheme.contains( serviceType ) )
        {
          QString serviceUrlResult = referenceNode.firstChild().nodeValue();
          return serviceUrlResult;
        }
      }
      else if ( serviceType == QStringLiteral( "XYZ" ) )
      {
        QDomNodeList titleNodeList = dom.elementsByTagName( "dc:title" );
        QString title;
        if ( !titleNodeList.isEmpty() )
        {
          QDomNode titleNode = titleNodeList.at( 0 );
          title = titleNode.firstChild().nodeValue();
        }
        QString serviceUrlResult = referenceNode.firstChild().nodeValue();
        if ( serviceUrlResult.contains( QStringLiteral( "{x}" ) ) && serviceUrlResult.contains( QStringLiteral( "{y}" ) ) && serviceUrlResult.contains( QStringLiteral( "{y}" ) ) )
        {
          // Hacky for QGIS Server backend
          if ( serviceUrlResult.contains( QStringLiteral( "qgis-server" ) ) )
          {
            serviceUrlResult = uri().param( "url" ) + QStringLiteral( "/qgis-server/tiles/LAYERNAME/{z}/{x}/{y}.png" );
            if ( !serviceUrlResult.contains( QLatin1String( "://" ) ) )
            {
              serviceUrlResult.prepend( "http://" );
            }
          }
          return serviceUrlResult;
        }
      }

    }
  }

  // return empty
  return QStringLiteral( "" );
}

QString QgsGeoNodeConnection::serviceUrl( QString serviceType )
{
  QString randomUUID = "";
  QVariantList layers = getLayers();

  if ( !layers.isEmpty() )
  {
    Q_FOREACH ( const QVariant &layer, layers )
    {
      randomUUID = layer.toMap()["uuid"].toString();
      QString url = serviceUrl( randomUUID, serviceType );
      if ( !url.isEmpty() )
      {
        return url;
      }
    }
  }
  return QStringLiteral( "" );
}
