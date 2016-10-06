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

#include <QSettings>

QString QgsXyzConnection::encodedUri() const
{
  QgsDataSourceUri uri;
  uri.setParam( "type", "xyz" );
  uri.setParam( "url", url );
  return uri.encodedUri();
}

QStringList QgsXyzConnectionUtils::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-xyz" );
  return settings.childGroups();
}

QgsXyzConnection QgsXyzConnectionUtils::connection( const QString &name )
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-xyz/" + name );

  QgsXyzConnection conn;
  conn.name = name;
  conn.url = settings.value( "url" ).toString();
  return conn;
}

void QgsXyzConnectionUtils::deleteConnection( const QString& name )
{
  QSettings settings;
  settings.remove( "/Qgis/connections-xyz/" + name );
}

void QgsXyzConnectionUtils::addConnection( const QgsXyzConnection &conn )
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-xyz/" + conn.name );
  settings.setValue( "url", conn.url );
}
