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
#include "qurl"

#include <QMultiMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QJsonDocument>
#include <QDebug>


const QString QgsGeoNodeConnection::pathGeoNodeConnection = "qgis/connections-geonode/";
const QString QgsGeoNodeConnection::pathGeoNodeConnectionDetails = "qgis/GeoNode/";

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
  settings.remove( pathGeoNodeConnection + name );
  settings.remove( pathGeoNodeConnectionDetails + name );
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
  // Construct URL. I need to prepend http in the begining to make it work.
  // setScheme doesn't really help.
  QString url = "http://" + uri().param( "url" ) + QStringLiteral( "/api/layers" );
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  // Handle redirect
  request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );

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

// Currently copy and paste from getLayers. It can be refactored easily, difference in url only.
QVariantList QgsGeoNodeConnection::getMaps()
{
  // Construct URL. I need to prepend http in the begining to make it work.
  // setScheme doesn't really help.
  QString url = "http://" + uri().param( "url" ) + QStringLiteral( "/api/maps" );
  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
  // Handle redirect
  request.setAttribute( QNetworkRequest::FollowRedirectsAttribute, true );

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
