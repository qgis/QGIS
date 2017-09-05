/***************************************************************************
    qgsgeonodeconnection.cpp
    ---------------------
    begin                : Feb 2017
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

#include "qgssettings.h"
#include "qgsgeonodeconnection.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"

const QString QgsGeoNodeConnection::sPathGeoNodeConnection = "qgis/connections-geonode";
const QString QgsGeoNodeConnection::sPathGeoNodeConnectionDetails = "qgis/GeoNode";

QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &connName )
  : mConnName( connName )
{
  QgsSettings settings;


//  settings.Section
  QString key = sPathGeoNodeConnection  + QStringLiteral( "/" ) + mConnName;
  QString credentialsKey = sPathGeoNodeConnectionDetails + QStringLiteral( "/" ) + mConnName;

  mUri.setParam( QStringLiteral( "url" ), settings.value( key + QStringLiteral( "/url" ), QString(), QgsSettings::Providers ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + QStringLiteral( "/username" ), QString(), QgsSettings::Providers ).toString();
  QString password = settings.value( credentialsKey + QStringLiteral( "/password" ), QString(), QgsSettings::Providers ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + QStringLiteral( "/authcfg" ), QString(), QgsSettings::Providers ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }

  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsGeoNodeConnection::~QgsGeoNodeConnection()
{

}

QgsDataSourceUri QgsGeoNodeConnection::uri() const
{
  return mUri;
}

QStringList QgsGeoNodeConnection::connectionList()
{
  QgsSettings settings;
  // Add Section manually
  settings.beginGroup( QStringLiteral( "providers/qgis/connections-geonode" ) );
  return settings.childGroups();
}

void QgsGeoNodeConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  // Add Section manually
  settings.remove( QStringLiteral( "providers/qgis/connections-geonode/" ) + name );
  settings.remove( QStringLiteral( "providers/qgis/geonode/" ) + name );
}

QString QgsGeoNodeConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/connections-geonode/selected" ), QString(), QgsSettings::Providers ).toString();
}

void QgsGeoNodeConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/connections-geonode/selected" ), name, QgsSettings::Providers );
}

QString QgsGeoNodeConnection::pathGeoNodeConnection()
{
  return sPathGeoNodeConnection;
}

QString QgsGeoNodeConnection::pathGeoNodeConnectionDetails()
{
  return sPathGeoNodeConnectionDetails;
}

QString QgsGeoNodeConnection::connName() const
{
  return mConnName;
}

void QgsGeoNodeConnection::setConnName( const QString &connName )
{
  mConnName = connName;
}

void QgsGeoNodeConnection::setUri( const QgsDataSourceUri &uri )
{
  mUri = uri;
}
