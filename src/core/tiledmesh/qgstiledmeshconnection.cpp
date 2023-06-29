/***************************************************************************
    qgstiledmeshconnection.cpp
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

#include "qgstiledmeshconnection.h"

#include "qgsdatasourceuri.h"
#include "qgshttpheaders.h"
#include "qgssettingsentryimpl.h"


#include <QFileInfo>

///@cond PRIVATE


const QgsSettingsEntryString *QgsTiledMeshProviderConnection::settingsProvider = new QgsSettingsEntryString( QStringLiteral( "provider" ), sTreeConnectionTiledMesh );
const QgsSettingsEntryString *QgsTiledMeshProviderConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeConnectionTiledMesh );
const QgsSettingsEntryString *QgsTiledMeshProviderConnection::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeConnectionTiledMesh );
const QgsSettingsEntryString *QgsTiledMeshProviderConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeConnectionTiledMesh );
const QgsSettingsEntryString *QgsTiledMeshProviderConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeConnectionTiledMesh );
const QgsSettingsEntryVariantMap *QgsTiledMeshProviderConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeConnectionTiledMesh );

///@endcond

QString QgsTiledMeshProviderConnection::encodedUri( const QgsTiledMeshProviderConnection::Data &data )
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

QgsTiledMeshProviderConnection::Data QgsTiledMeshProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsTiledMeshProviderConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.httpHeaders = dsUri.httpHeaders();

  return conn;
}

QString QgsTiledMeshProviderConnection::encodedLayerUri( const QgsTiledMeshProviderConnection::Data &data )
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

QStringList QgsTiledMeshProviderConnection::connectionList()
{
  return QgsTiledMeshProviderConnection::sTreeConnectionTiledMesh->items();
}

QgsTiledMeshProviderConnection::Data QgsTiledMeshProviderConnection::connection( const QString &name )
{
  if ( !settingsUrl->exists( name ) )
    return QgsTiledMeshProviderConnection::Data();

  QgsTiledMeshProviderConnection::Data conn;
  conn.provider = settingsProvider->value( name );
  conn.url = settingsUrl->value( name );
  conn.authCfg = settingsAuthcfg->value( name );
  conn.username = settingsUsername->value( name );
  conn.password = settingsPassword->value( name );

  if ( settingsHeaders->exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders->value( name ) );

  return conn;
}

void QgsTiledMeshProviderConnection::addConnection( const QString &name, const Data &conn )
{
  settingsProvider->setValue( conn.provider, name );
  settingsUrl->setValue( conn.url, name );
  settingsAuthcfg->setValue( conn.authCfg, name );
  settingsUsername->setValue( conn.username, name );
  settingsPassword->setValue( conn.password, name );
  settingsHeaders->setValue( conn.httpHeaders.headers(), name );
}

QString QgsTiledMeshProviderConnection::selectedConnection()
{
  return sTreeConnectionTiledMesh->selectedItem();
}

void QgsTiledMeshProviderConnection::setSelectedConnection( const QString &name )
{
  sTreeConnectionTiledMesh->setSelectedItem( name );
}

QgsTiledMeshProviderConnection::QgsTiledMeshProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  const QgsTiledMeshProviderConnection::Data connectionData = connection( name );
  mProvider = connectionData.provider;
  setUri( encodedUri( connectionData ) );
}

QgsTiledMeshProviderConnection::QgsTiledMeshProviderConnection( const QString &uri, const QString &provider, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
  , mProvider( provider )
{
}

void QgsTiledMeshProviderConnection::store( const QString &name ) const
{
  QgsTiledMeshProviderConnection::Data connectionData = decodedUri( uri() );
  connectionData.provider = mProvider;
  addConnection( name, connectionData );
}

void QgsTiledMeshProviderConnection::remove( const QString &name ) const
{
  sTreeConnectionTiledMesh->deleteItem( name );
}
