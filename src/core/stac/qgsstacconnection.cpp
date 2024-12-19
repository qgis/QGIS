/***************************************************************************
    qgsstacconnection.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacconnection.h"

#include "qgsdatasourceuri.h"
#include "qgssettingsentryimpl.h"


///@cond PRIVATE

const QgsSettingsEntryString *QgsStacConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeConnectionStac );
const QgsSettingsEntryString *QgsStacConnection::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeConnectionStac );
const QgsSettingsEntryString *QgsStacConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeConnectionStac );
const QgsSettingsEntryString *QgsStacConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeConnectionStac );
const QgsSettingsEntryVariantMap *QgsStacConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeConnectionStac );


QString QgsStacConnection::encodedUri( const QgsStacConnection::Data &conn )
{
  QgsDataSourceUri uri;

  uri.setParam( QStringLiteral( "url" ), conn.url );
  if ( !conn.authCfg.isEmpty() )
    uri.setAuthConfigId( conn.authCfg );
  if ( !conn.username.isEmpty() )
    uri.setUsername( conn.username );
  if ( !conn.password.isEmpty() )
    uri.setPassword( conn.password );

  uri.setHttpHeaders( conn.httpHeaders );

  return uri.encodedUri();
}

QgsStacConnection::Data QgsStacConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsStacConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.httpHeaders = dsUri.httpHeaders();

  return conn;
}

QStringList QgsStacConnection::connectionList()
{
  return QgsStacConnection::sTreeConnectionStac->items();
}

QgsStacConnection::Data QgsStacConnection::connection( const QString &name )
{
  if ( !settingsUrl->exists( name ) )
    return QgsStacConnection::Data();

  QgsStacConnection::Data conn;
  conn.url = settingsUrl->value( name );
  conn.authCfg = settingsAuthcfg->value( name );
  conn.username = settingsUsername->value( name );
  conn.password = settingsPassword->value( name );

  if ( settingsHeaders->exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders->value( name ) );

  return conn;
}

void QgsStacConnection::deleteConnection( const QString &name )
{
  sTreeConnectionStac->deleteItem( name );
}

void QgsStacConnection::addConnection( const QString &name, const Data &conn )
{
  settingsUrl->setValue( conn.url, name );
  settingsAuthcfg->setValue( conn.authCfg, name );
  settingsUsername->setValue( conn.username, name );
  settingsPassword->setValue( conn.password, name );
  settingsHeaders->setValue( conn.httpHeaders.headers(), name );
}

QString QgsStacConnection::selectedConnection()
{
  return sTreeConnectionStac->selectedItem();
}

void QgsStacConnection::setSelectedConnection( const QString &name )
{
  sTreeConnectionStac->setSelectedItem( name );
}


QgsStacConnection::QgsStacConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  setUri( encodedUri( connection( name ) ) );
}

QgsStacConnection::QgsStacConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
{
}

void QgsStacConnection::store( const QString &name ) const
{
  addConnection( name, decodedUri( uri() ) );
}

void QgsStacConnection::remove( const QString &name ) const
{
  deleteConnection( name );
}

///@endcond
