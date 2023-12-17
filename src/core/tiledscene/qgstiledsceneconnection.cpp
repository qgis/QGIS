/***************************************************************************
    qgstiledsceneconnection.cpp
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneconnection.h"

#include "qgsdatasourceuri.h"
#include "qgshttpheaders.h"
#include "qgssettingsentryimpl.h"


#include <QFileInfo>

///@cond PRIVATE


const QgsSettingsEntryString *QgsTiledSceneProviderConnection::settingsProvider = new QgsSettingsEntryString( QStringLiteral( "provider" ), sTreeConnectionTiledScene );
const QgsSettingsEntryString *QgsTiledSceneProviderConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeConnectionTiledScene );
const QgsSettingsEntryString *QgsTiledSceneProviderConnection::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeConnectionTiledScene );
const QgsSettingsEntryString *QgsTiledSceneProviderConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeConnectionTiledScene );
const QgsSettingsEntryString *QgsTiledSceneProviderConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeConnectionTiledScene );
const QgsSettingsEntryVariantMap *QgsTiledSceneProviderConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeConnectionTiledScene );

///@endcond

QString QgsTiledSceneProviderConnection::encodedUri( const QgsTiledSceneProviderConnection::Data &data )
{
  QgsDataSourceUri uri;

  if ( !data.url.isEmpty() )
    uri.setParam( QStringLiteral( "url" ), data.url );
  if ( !data.authCfg.isEmpty() )
    uri.setAuthConfigId( data.authCfg );
  if ( !data.username.isEmpty() )
    uri.setUsername( data.username );
  if ( !data.password.isEmpty() )
    uri.setPassword( data.password );

  uri.setHttpHeaders( data.httpHeaders );

  return uri.encodedUri();
}

QgsTiledSceneProviderConnection::Data QgsTiledSceneProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsTiledSceneProviderConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.httpHeaders = dsUri.httpHeaders();

  return conn;
}

QString QgsTiledSceneProviderConnection::encodedLayerUri( const QgsTiledSceneProviderConnection::Data &data )
{
  QgsDataSourceUri uri;

  uri.setParam( QStringLiteral( "url" ), data.url );
  if ( !data.authCfg.isEmpty() )
    uri.setAuthConfigId( data.authCfg );
  if ( !data.username.isEmpty() )
    uri.setUsername( data.username );
  if ( !data.password.isEmpty() )
    uri.setPassword( data.password );

  uri.setHttpHeaders( data.httpHeaders );

  return uri.encodedUri();
}

QStringList QgsTiledSceneProviderConnection::connectionList()
{
  return QgsTiledSceneProviderConnection::sTreeConnectionTiledScene->items();
}

QgsTiledSceneProviderConnection::Data QgsTiledSceneProviderConnection::connection( const QString &name )
{
  if ( !settingsUrl->exists( name ) )
    return QgsTiledSceneProviderConnection::Data();

  QgsTiledSceneProviderConnection::Data conn;
  conn.provider = settingsProvider->value( name );
  conn.url = settingsUrl->value( name );
  conn.authCfg = settingsAuthcfg->value( name );
  conn.username = settingsUsername->value( name );
  conn.password = settingsPassword->value( name );

  if ( settingsHeaders->exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders->value( name ) );

  return conn;
}

void QgsTiledSceneProviderConnection::addConnection( const QString &name, const Data &conn )
{
  settingsProvider->setValue( conn.provider, name );
  settingsUrl->setValue( conn.url, name );
  settingsAuthcfg->setValue( conn.authCfg, name );
  settingsUsername->setValue( conn.username, name );
  settingsPassword->setValue( conn.password, name );
  settingsHeaders->setValue( conn.httpHeaders.headers(), name );
}

QString QgsTiledSceneProviderConnection::selectedConnection()
{
  return sTreeConnectionTiledScene->selectedItem();
}

void QgsTiledSceneProviderConnection::setSelectedConnection( const QString &name )
{
  sTreeConnectionTiledScene->setSelectedItem( name );
}

QgsTiledSceneProviderConnection::QgsTiledSceneProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  const QgsTiledSceneProviderConnection::Data connectionData = connection( name );
  mProvider = connectionData.provider;
  setUri( encodedUri( connectionData ) );
}

QgsTiledSceneProviderConnection::QgsTiledSceneProviderConnection( const QString &uri, const QString &provider, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
  , mProvider( provider )
{
}

void QgsTiledSceneProviderConnection::store( const QString &name ) const
{
  QgsTiledSceneProviderConnection::Data connectionData = decodedUri( uri() );
  connectionData.provider = mProvider;
  addConnection( name, connectionData );
}

void QgsTiledSceneProviderConnection::remove( const QString &name ) const
{
  sTreeConnectionTiledScene->deleteItem( name );
}
