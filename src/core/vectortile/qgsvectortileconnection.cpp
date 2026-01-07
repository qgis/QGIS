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

#include "qgsdatasourceuri.h"
#include "qgshttpheaders.h"
#include "qgssettingsentryimpl.h"

#include <QFileInfo>

///@cond PRIVATE

const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsUrl = new QgsSettingsEntryString( u"url"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryInteger *QgsVectorTileProviderConnection::settingsZmin = new QgsSettingsEntryInteger( u"zmin"_s, sTreeConnectionVectorTile, -1 );
const QgsSettingsEntryInteger *QgsVectorTileProviderConnection::settingsZmax = new QgsSettingsEntryInteger( u"zmax"_s, sTreeConnectionVectorTile, -1 );
const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsAuthcfg = new QgsSettingsEntryString( u"authcfg"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsUsername = new QgsSettingsEntryString( u"username"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsPassword = new QgsSettingsEntryString( u"password"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsStyleUrl = new QgsSettingsEntryString( u"styleUrl"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryString *QgsVectorTileProviderConnection::settingsServiceType = new QgsSettingsEntryString( u"service-type"_s, sTreeConnectionVectorTile );
const QgsSettingsEntryVariantMap *QgsVectorTileProviderConnection::settingsHeaders = new QgsSettingsEntryVariantMap( u"http-header"_s, sTreeConnectionVectorTile );


QString QgsVectorTileProviderConnection::encodedUri( const QgsVectorTileProviderConnection::Data &conn )
{
  QgsDataSourceUri uri;

  const QFileInfo info( conn.url );
  QString suffix = info.suffix().toLower();
  if ( suffix.startsWith( "mbtiles"_L1 ) )
  {
    uri.setParam( u"type"_s, u"mbtiles"_s );
  }
  else
  {
    uri.setParam( u"type"_s, u"xyz"_s );
  }

  uri.setParam( u"url"_s, conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( u"zmin"_s, QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( u"zmax"_s, QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( u"styleUrl"_s,  conn.styleUrl );

  uri.setHttpHeaders( conn.httpHeaders );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( u"serviceType"_s, u"arcgis"_s );
      break;
  }

  return uri.encodedUri();
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsVectorTileProviderConnection::Data conn;
  conn.url = dsUri.param( u"url"_s );
  conn.zMin = dsUri.hasParam( u"zmin"_s ) ? dsUri.param( u"zmin"_s ).toInt() : -1;
  conn.zMax = dsUri.hasParam( u"zmax"_s ) ? dsUri.param( u"zmax"_s ).toInt() : -1;
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.styleUrl = dsUri.param( u"styleUrl"_s );

  conn.httpHeaders = dsUri.httpHeaders();

  if ( dsUri.hasParam( u"serviceType"_s ) )
  {
    if ( dsUri.param( u"serviceType"_s ) == "arcgis"_L1 )
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
  if ( suffix.startsWith( "mbtiles"_L1 ) )
  {
    uri.setParam( u"type"_s, u"mbtiles"_s );
  }
  else
  {
    uri.setParam( u"type"_s, u"xyz"_s );
  }

  uri.setParam( u"url"_s, conn.url );
  if ( conn.zMin != -1 )
    uri.setParam( u"zmin"_s, QString::number( conn.zMin ) );
  if ( conn.zMax != -1 )
    uri.setParam( u"zmax"_s, QString::number( conn.zMax ) );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );
  if ( !conn.styleUrl.isEmpty() )
    uri.setParam( u"styleUrl"_s,  conn.styleUrl );

  uri.setHttpHeaders( conn.httpHeaders );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      uri.setParam( u"serviceType"_s, u"arcgis"_s );
      break;
  }

  return uri.encodedUri();
}

QStringList QgsVectorTileProviderConnection::connectionList()
{
  return QgsVectorTileProviderConnection::sTreeConnectionVectorTile->items();
}

QgsVectorTileProviderConnection::Data QgsVectorTileProviderConnection::connection( const QString &name )
{
  if ( !settingsUrl->exists( name ) && !settingsStyleUrl->exists( name ) )
    return QgsVectorTileProviderConnection::Data();

  QgsVectorTileProviderConnection::Data conn;
  conn.url = settingsUrl->value( name );
  conn.zMin = settingsZmin->value( name );
  conn.zMax = settingsZmax->value( name );
  conn.authCfg = settingsAuthcfg->value( name );
  conn.username = settingsUsername->value( name );
  conn.password = settingsPassword->value( name );
  conn.styleUrl = settingsStyleUrl->value( name );

  if ( settingsHeaders->exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders->value( name ) );

  if ( settingsServiceType->exists( name ) &&  settingsServiceType->value( name ) == "arcgis"_L1 )
    conn.serviceType = ArcgisVectorTileService;

  return conn;
}

void QgsVectorTileProviderConnection::deleteConnection( const QString &name )
{
  sTreeConnectionVectorTile->deleteItem( name );
}

void QgsVectorTileProviderConnection::addConnection( const QString &name, QgsVectorTileProviderConnection::Data conn )
{
  settingsUrl->setValue( conn.url, name );
  settingsZmin->setValue( conn.zMin, name );
  settingsZmax->setValue( conn.zMax, name );
  settingsAuthcfg->setValue( conn.authCfg, name );
  settingsUsername->setValue( conn.username, name );
  settingsPassword->setValue( conn.password, name );
  settingsStyleUrl->setValue( conn.styleUrl, name );

  settingsHeaders->setValue( conn.httpHeaders.headers(), name );

  switch ( conn.serviceType )
  {
    case Generic:
      break;

    case ArcgisVectorTileService:
      settingsServiceType->setValue( u"arcgis"_s, name );
      break;
  }
}

QString QgsVectorTileProviderConnection::selectedConnection()
{
  return sTreeConnectionVectorTile->selectedItem();
}

void QgsVectorTileProviderConnection::setSelectedConnection( const QString &name )
{
  sTreeConnectionVectorTile->setSelectedItem( name );
}


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
