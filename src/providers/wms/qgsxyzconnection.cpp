/***************************************************************************
    qgsxyzconnection.h
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsxyzconnection.h"

#include "qgsdatasourceuri.h"
#include "qgssettings.h"

QString QgsXyzConnection::encodedUri() const
{
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uri.setParam( QStringLiteral( "url" ), url );
  if ( zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( zMin ) );
  if ( zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( zMax ) );
  if ( ! authCfg.isEmpty() )
    uri.setParam( QStringLiteral( "authcfg" ), authCfg );
  if ( ! username.isEmpty() )
    uri.setParam( QStringLiteral( "username" ), username );
  if ( ! password.isEmpty() )
    uri.setParam( QStringLiteral( "password" ), password );
  if ( ! referer.isEmpty() )
    uri.setParam( QStringLiteral( "referer" ), referer );
  return uri.encodedUri();
}

QStringList QgsXyzConnectionUtils::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
  return settings.childGroups();
}

QgsXyzConnection QgsXyzConnectionUtils::connection( const QString &name )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-xyz/" + name );

  QgsXyzConnection conn;
  conn.name = name;
  conn.url = settings.value( QStringLiteral( "url" ) ).toString();
  conn.zMin = settings.value( QStringLiteral( "zmin" ), -1 ).toInt();
  conn.zMax = settings.value( QStringLiteral( "zmax" ), -1 ).toInt();
  conn.authCfg = settings.value( QStringLiteral( "authcfg" ) ).toString();
  conn.username = settings.value( QStringLiteral( "username" ) ).toString();
  conn.password = settings.value( QStringLiteral( "password" ) ).toString();
  conn.referer = settings.value( QStringLiteral( "referer" ) ).toString();
  return conn;
}

void QgsXyzConnectionUtils::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-xyz/" + name );
}

void QgsXyzConnectionUtils::addConnection( const QgsXyzConnection &conn )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-xyz/" + conn.name );
  settings.setValue( QStringLiteral( "url" ), conn.url );
  settings.setValue( QStringLiteral( "zmin" ), conn.zMin );
  settings.setValue( QStringLiteral( "zmax" ), conn.zMax );
  settings.setValue( QStringLiteral( "authcfg" ), conn.authCfg );
  settings.setValue( QStringLiteral( "username" ), conn.username );
  settings.setValue( QStringLiteral( "password" ), conn.password );
  settings.setValue( QStringLiteral( "referer" ), conn.referer );
}
