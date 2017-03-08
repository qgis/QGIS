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
  return uri.encodedUri();
}

QStringList QgsXyzConnectionUtils::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "Qgis/connections-xyz" ) );
  return settings.childGroups();
}

QgsXyzConnection QgsXyzConnectionUtils::connection( const QString &name )
{
  QgsSettings settings;
  settings.beginGroup( "/Qgis/connections-xyz/" + name );

  QgsXyzConnection conn;
  conn.name = name;
  conn.url = settings.value( QStringLiteral( "url" ) ).toString();
  conn.zMin = settings.value( QStringLiteral( "zmin" ), -1 ).toInt();
  conn.zMax = settings.value( QStringLiteral( "zmax" ), -1 ).toInt();
  return conn;
}

void QgsXyzConnectionUtils::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "/Qgis/connections-xyz/" + name );
}

void QgsXyzConnectionUtils::addConnection( const QgsXyzConnection &conn )
{
  QgsSettings settings;
  settings.beginGroup( "/Qgis/connections-xyz/" + conn.name );
  settings.setValue( QStringLiteral( "url" ), conn.url );
  settings.setValue( QStringLiteral( "zmin" ), conn.zMin );
  settings.setValue( QStringLiteral( "zmax" ), conn.zMax );
}
