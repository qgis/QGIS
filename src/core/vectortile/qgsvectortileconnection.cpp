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
#include "qgshttpheaders.h"

#include <QFileInfo>

///@cond PRIVATE

QString QgsVectorTileProviderConnection::encodedUri( const QgsVectorTileProviderConnection::Data &conn )
{
  QgsDataSourceUri uri;

  const QFileInfo info( conn.url );
  QString suffix = info.suffix().toLower();
  if ( suffix.startsWith( QLatin1String( "mbtiles" ) ) )
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  }
  else
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  }

  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.referer.isEmpty() )
    uri.setParam( QStringLiteral( "referer" ),  conn.referer );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( QStringLiteral( "styleUrl" ),  conn.styleUrl );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
      break;
  }

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
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.referer = dsUri.param( QStringLiteral( "referer" ) );
  conn.styleUrl = dsUri.param( QStringLiteral( "styleUrl" ) );

  if ( dsUri.hasParam( QStringLiteral( "serviceType" ) ) )
  {
    if ( dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
      conn.serviceType = ArcgisVectorTileService;
  }
  return conn;
}

QString QgsVectorTileProviderConnection::encodedLayerUri( const QgsVectorTileProviderConnection::Data &conn )
{
  // compared to encodedUri() this one also adds type=xyz to the URI
  QgsDataSourceUri uri;

  const QFileInfo info( conn.url );
  QString suffix = info.suffix().toLower();
  if ( suffix.startsWith( QLatin1String( "mbtiles" ) ) )
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "mbtiles" ) );
  }
  else
  {
    uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  }

  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.referer.isEmpty() )
    uri.setParam( QStringLiteral( "referer" ),  conn.referer );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( QStringLiteral( "styleUrl" ),  conn.styleUrl );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
      break;
  }

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
  conn.authCfg = settings.value( QStringLiteral( "authcfg" ) ).toString();
  conn.username = settings.value( QStringLiteral( "username" ) ).toString();
  conn.password = settings.value( QStringLiteral( "password" ) ).toString();
  conn.referer = QgsHttpHeaders( settings )[ QStringLiteral( "referer" ) ].toString();
  conn.styleUrl = settings.value( QStringLiteral( "styleUrl" ) ).toString();

  if ( settings.contains( QStringLiteral( "serviceType" ) ) )
  {
    if ( settings.value( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
      conn.serviceType = ArcgisVectorTileService;
  }

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
  settings.setValue( QStringLiteral( "authcfg" ), conn.authCfg );
  settings.setValue( QStringLiteral( "username" ), conn.username );
  settings.setValue( QStringLiteral( "password" ), conn.password );
  QgsHttpHeaders( QVariantMap( { {QStringLiteral( "referer" ), conn.referer}} ) ).updateSettings( settings );
  settings.setValue( QStringLiteral( "styleUrl" ), conn.styleUrl );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      settings.setValue( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
      break;
  }
}

QString QgsVectorTileProviderConnection::selectedConnection()
{
  const QgsSettings settings;
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
