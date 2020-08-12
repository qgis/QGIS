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

QString QgsVectorTileProviderConnection::encodedUri( const QgsVectorTileProviderConnection::Data &conn )
{
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  return uri.encodedUri();
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsVectorTileProviderConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.zMin = dsUri.hasParam( QStringLiteral( "zmin" ) ) ? dsUri.param( QStringLiteral( "zmin" ) ).toInt() : -1;
  conn.zMax = dsUri.hasParam( QStringLiteral( "zmax" ) ) ? dsUri.param( QStringLiteral( "zmax" ) ).toInt() : -1;
  return conn;
}

QString QgsVectorTileProviderConnection::encodedLayerUri( const QgsVectorTileProviderConnection::Data &conn )
{
  // compared to encodedUri() this one also adds type=xyz to the URI
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  return uri.encodedUri();
}

QStringList QgsVectorTileProviderConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-vector-tile" ) );
  QStringList connList = settings.childGroups();

  return connList;
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::connection( const QString &name )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-vector-tile/" + name );

  if ( settings.value( "url" ).toString().isEmpty() )
    return QgsVectorTileProviderConnection::Data();

  QgsVectorTileProviderConnection::Data conn;
  conn.url = settings.value( QStringLiteral( "url" ) ).toString();
  conn.zMin = settings.value( QStringLiteral( "zmin" ), -1 ).toInt();
  conn.zMax = settings.value( QStringLiteral( "zmax" ), -1 ).toInt();
  return conn;
}

void QgsVectorTileProviderConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-vector-tile/" + name );
}

void QgsVectorTileProviderConnection::addConnection( const QString &name, QgsVectorTileProviderConnection::Data conn )
{
  QgsSettings settings;

  settings.beginGroup( "qgis/connections-vector-tile/" + name );
  settings.setValue( QStringLiteral( "url" ), conn.url );
  settings.setValue( QStringLiteral( "zmin" ), conn.zMin );
  settings.setValue( QStringLiteral( "zmax" ), conn.zMax );
}

QString QgsVectorTileProviderConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/connections-vector-tile/selected" ) ).toString();
}

void QgsVectorTileProviderConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( QStringLiteral( "qgis/connections-vector-tile/selected" ), name );
}

//

QgsVectorTileProviderConnection::QgsVectorTileProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  setUri( encodedUri( connection( name ) ) );
}

QgsVectorTileProviderConnection::QgsVectorTileProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
{
}

void QgsVectorTileProviderConnection::store( const QString &name ) const
{
  addConnection( name, decodedUri( uri() ) );
}

void QgsVectorTileProviderConnection::remove( const QString &name ) const
{
  deleteConnection( name );
}

///@endcond
