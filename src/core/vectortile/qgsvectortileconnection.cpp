/***************************************************************************
    qgsvectortileconnection.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileconnection.h"

#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgssettings.h"

///@cond PRIVATE

QString QgsVectorTileConnection::encodedUri() const
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

QStringList QgsVectorTileConnectionUtils::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );
  QStringList connList = settings.childGroups();

  return connList;
}

QgsVectorTileConnection QgsVectorTileConnectionUtils::connection( const QString &name )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-vector-tile/" + name );

  QgsVectorTileConnection conn;
  conn.name = name;
  conn.url = settings.value( QStringLiteral( "url" ) ).toString();
  conn.zMin = settings.value( QStringLiteral( "zmin" ), -1 ).toInt();
  conn.zMax = settings.value( QStringLiteral( "zmax" ), -1 ).toInt();
  return conn;
}

void QgsVectorTileConnectionUtils::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-vector-tile/" + name );
}

void QgsVectorTileConnectionUtils::addConnection( const QgsVectorTileConnection &conn )
{
  QgsSettings settings;

  settings.beginGroup( "qgis/connections-vector-tile/" + conn.name );
  settings.setValue( QStringLiteral( "url" ), conn.url );
  settings.setValue( QStringLiteral( "zmin" ), conn.zMin );
  settings.setValue( QStringLiteral( "zmax" ), conn.zMax );
}

///@endcond
