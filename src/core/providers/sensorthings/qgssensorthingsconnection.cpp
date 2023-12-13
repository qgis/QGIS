/***************************************************************************
    QgsSensorThingsProviderConnection.cpp
    ---------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgslogger.h>
#include "qgssensorthingsconnection.h"

#include "qgsowsconnection.h"
#include "qgsdatasourceuri.h"
#include "qgssettingsentryimpl.h"

///@cond PRIVATE
const QgsSettingsEntryString *QgsSensorThingsProviderConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeSensorThingsConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsSensorThingsProviderConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsProviderConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsProviderConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsProviderConnection::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeSensorThingsConnections ) ;
///@endcond



QgsSensorThingsProviderConnection::QgsSensorThingsProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  const QgsSensorThingsProviderConnection::Data connectionData = connection( name );
  setUri( encodedUri( connectionData ) );
}

QgsSensorThingsProviderConnection::QgsSensorThingsProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
{
}

void QgsSensorThingsProviderConnection::store( const QString &name ) const
{
  QgsSensorThingsProviderConnection::Data connectionData = decodedUri( uri() );
  addConnection( name, connectionData );
}

void QgsSensorThingsProviderConnection::remove( const QString &name ) const
{
  sTreeSensorThingsConnections->deleteItem( name );
}

QString QgsSensorThingsProviderConnection::selectedConnection()
{
  return sTreeSensorThingsConnections->selectedItem();
}

void QgsSensorThingsProviderConnection::setSelectedConnection( const QString &name )
{
  sTreeSensorThingsConnections->setSelectedItem( name );
}

void QgsSensorThingsProviderConnection::addConnection( const QString &name, const Data &conn )
{
  settingsUrl->setValue( conn.url, name );
  settingsAuthcfg->setValue( conn.authCfg, name );
  settingsUsername->setValue( conn.username, name );
  settingsPassword->setValue( conn.password, name );
  settingsHeaders->setValue( conn.httpHeaders.headers(), name );
}

QgsSensorThingsProviderConnection::Data QgsSensorThingsProviderConnection::connection( const QString &name )
{
  if ( !settingsUrl->exists( name ) )
    return QgsSensorThingsProviderConnection::Data();

  QgsSensorThingsProviderConnection::Data conn;
  conn.url = settingsUrl->value( name );
  conn.authCfg = settingsAuthcfg->value( name );
  conn.username = settingsUsername->value( name );
  conn.password = settingsPassword->value( name );

  if ( settingsHeaders->exists( name ) )
    conn.httpHeaders = QgsHttpHeaders( settingsHeaders->value( name ) );

  return conn;
}

QStringList QgsSensorThingsProviderConnection::connectionList()
{
  return QgsSensorThingsProviderConnection::sTreeSensorThingsConnections->items();
}

QString QgsSensorThingsProviderConnection::encodedUri( const QgsSensorThingsProviderConnection::Data &data )
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

QgsSensorThingsProviderConnection::Data QgsSensorThingsProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsSensorThingsProviderConnection::Data conn;
  conn.url = dsUri.param( QStringLiteral( "url" ) );
  conn.authCfg = dsUri.authConfigId();
  conn.username = dsUri.username();
  conn.password = dsUri.password();
  conn.httpHeaders = dsUri.httpHeaders();

  return conn;
}

QString QgsSensorThingsProviderConnection::encodedLayerUri( const QgsSensorThingsProviderConnection::Data &data )
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

  return uri.uri( false );
}

