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


QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &connName )
  : mConnName( connName )
{
  QgsDebugMsg( "theConnName = " + connName );

  QgsSettings settings;

  QString key = "qgis/connections-geonode/" + mConnName;
  QString credentialsKey = "qgis/GeoNode/" + mConnName;

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
  settings.beginGroup( QStringLiteral( "qgis/connections-geonode" ) );
  return settings.childGroups();
}

void QgsGeoNodeConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-geonode/" + name );
  settings.remove( "qgis/GeoNode/" + name );
}

QString QgsGeoNodeConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/connections-geonode/selected" ) ).toString();
}

void QgsGeoNodeConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/connections-geonode/selected" ), name );
}


