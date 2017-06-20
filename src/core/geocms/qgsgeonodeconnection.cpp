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
#include "qgsgeocmsconnection.h"
#include "qgsexception.h"

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
  : QgsGeoCMSConnection( QStringLiteral( "GeoNode" ), connName )
{
  QgsDebugMsg( "theConnName = " + connName );
  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsGeoNodeConnection::~QgsGeoNodeConnection()
{

}

QStringList QgsGeoNodeConnection::connectionList()
{
  return QgsGeoCMSConnection::connectionList( QStringLiteral( "GeoNode" ) );
}

void QgsGeoNodeConnection::deleteConnection( const QString &name )
{
  return QgsGeoCMSConnection::deleteConnection( QStringLiteral( "GeoNode" ), name );
}

QString QgsGeoNodeConnection::selectedConnection()
{
  return QgsGeoCMSConnection::selectedConnection( QStringLiteral( "GeoNode" ) );
}

void QgsGeoNodeConnection::setSelectedConnection( const QString &name )
{
  return QgsGeoCMSConnection::setSelectedConnection( QStringLiteral( "GeoNode" ), name );
}

QVariantList QgsGeoNodeConnection::getLayers()
{
  QString url = uri().param( "url" ) + QStringLiteral( "/api/layers/" );
  QString protocol = url.split( "://" )[0];
  QUrl layerUrl( url );
  layerUrl.setScheme( protocol );
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

      // Set name and typename
      layer["name"] = layerName;
      layer["typename"] = layerTypeName;

      // WMS url : BASE_URI/geoserver/WORKSPACE/wms
      layer["wms"] = uri().param( "url" ) + "/geoserver/" + layerWorkspace + "/wms";
      // WFS url : BASE_URI/geoserver/WORKSPACE/wfs
      layer["wfs"] = uri().param( "url" ) + "/geoserver/" + layerWorkspace + "/wfs";
      // XYZ url : set to empty string
      layer["xyz"] = "";

      layerList[i] = layer;
    }
  }
  // Geonode version 2.7 or newer
  else if ( ( majorVersion == 2 && minorVersion >= 7 ) || ( majorVersion >= 3 ) )
  {
    for ( int i = 0; i < layerList.count(); i++ )
    {
      QVariantMap layer = layerList[i].toMap();
      // Find WMS, WFS, and XYZ link
      QVariantList layerLinks = layer["links"].toList();
      layer["wms"] = QStringLiteral( "" );
      layer["wfs"] = QStringLiteral( "" );
      layer["xyz"] = QStringLiteral( "" );
      for ( int j = 0; j < layerLinks.count(); j++ )
      {
        QVariantMap link = layerLinks[j].toMap();
        if ( link.contains( "link_type" ) )
        {
          if ( link["link_type"] == "OGC:WMS" )
          {
            layer["wms"] = link["url"].toString();
          }
          if ( link["link_type"] == "OGC:WFS" )
          {
            layer["wfs"] = link["url"].toString();
          }
          if ( link["link_type"] == "image" )
          {
            if ( link.contains( "name" ) && link["name"] == "Tiles" )
            {
              layer["xyz"] = link["url"].toString();
            }
          }
        }
      }
      if ( layer["typename"].toString().length() == 0 )
      {
        QStringList splitURL = layer["detail_url"].toString().split( "/" );

        layer["typename"] = splitURL[ splitURL.length() - 1];
      }
      layerList[i] = layer;
    }
  }
  return layerList;
}

QVariantList QgsGeoNodeConnection::getLayers( QString serviceType )
{
  QString param = QString( "?version=2.0.0&service=%1&request=GetCapabilities" ).arg( serviceType.toLower() );
  QString url = serviceUrl( serviceType )[0] + param;
  QUrl layerUrl( url );

  QString protocol = url.split( "://" )[0];
  layerUrl.setScheme( protocol );
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
  QString url = uri().param( "url" ) + QStringLiteral( "/api/maps/" );
  QString protocol = url.split( "://" )[0];
  QUrl layerUrl( url );
  layerUrl.setScheme( protocol );

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

QStringList QgsGeoNodeConnection::serviceUrl( QString &resourceID, QString serviceType )
{
  // Example CSW url
  // demo.geonode.org/catalogue/csw?request=GetRecordById&service=CSW&version=2.0.2&elementSetName=full&id=

  QString url = uri().param( "url" ) + QString( "/catalogue/csw?request=GetRecordById&service=CSW&version=2.0.2&elementSetName=full&id=%1" ).arg( resourceID );
  QUrl layerUrl( url );
  QString protocol = url.split( "://" )[0];
  layerUrl.setScheme( protocol );
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
          return QStringList( serviceUrlResult );
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
          }
          return QStringList( serviceUrlResult );
        }
      }

    }
  }

  // return empty
  return QStringList();
}

QStringList QgsGeoNodeConnection::serviceUrl( QString serviceType )
{
  QVariantList layers = getLayers();
  QStringList *urls = new QStringList;

  for ( int i = 0; i < layers.count(); i++ )
  {
    QString url = layers[i].toMap()[serviceType.toLower()].toString();
    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      url.prepend( "http://" );
    }
    if ( !urls->contains( url ) && url.length() > 0 )
    {
      urls->append( url );
    }
  }

  return *urls;
}

QVariantMap QgsGeoNodeConnection::serviceUrlData( QString serviceType )
{
  QVariantList layers = getLayers();
  QVariantMap *urls = new QVariantMap;

  for ( int i = 0; i < layers.count(); i++ )
  {
    QString url = layers[i].toMap()[serviceType.toLower()].toString();
    QString layerName = layers[i].toMap()["name"].toString();
    if ( !url.contains( QLatin1String( "://" ) ) && url.length() > 0 )
    {
      url.prepend( "http://" );
    }
    if ( !urls->contains( url ) && url.length() > 0 )
    {
      urls->insert( layerName, url );
    }
  }

  return *urls;
}
