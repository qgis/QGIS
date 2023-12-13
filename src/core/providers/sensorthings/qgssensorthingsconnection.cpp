/***************************************************************************
    qgssensorthingsconnection.cpp
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


const QgsSettingsEntryString *QgsSensorThingsConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeSensorThingsConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsSensorThingsConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeSensorThingsConnections ) ;
const QgsSettingsEntryString *QgsSensorThingsConnection::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeSensorThingsConnections ) ;

QString QgsSensorThingsConnection::encodedUri() const
{
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "url" ), url );
  if ( ! authCfg.isEmpty() )
    uri.setAuthConfigId( authCfg );
  if ( ! username.isEmpty() )
    uri.setUsername( username );
  if ( ! password.isEmpty() )
    uri.setPassword( password );

  uri.setHttpHeaders( httpHeaders );
  return uri.uri( false );
}

QStringList QgsSensorThingsConnectionUtils::connectionList()
{
  return QgsSensorThingsConnection::sTreeSensorThingsConnections->items();
}

QgsSensorThingsConnection QgsSensorThingsConnectionUtils::connection( const QString &name )
{
  QgsSensorThingsConnection conn;
  conn.name = name;
  conn.url = QgsSensorThingsConnection::settingsUrl->value( name );
  conn.authCfg = QgsSensorThingsConnection::settingsAuthcfg->value( name );
  conn.username = QgsSensorThingsConnection::settingsUsername->value( name );
  conn.password = QgsSensorThingsConnection::settingsPassword->value( name );
  conn.httpHeaders = QgsSensorThingsConnection::settingsHeaders->value( name );
  return conn;
}

void QgsSensorThingsConnectionUtils::deleteConnection( const QString &name )
{
  QgsSensorThingsConnection::sTreeSensorThingsConnections->deleteItem( name );
}

void QgsSensorThingsConnectionUtils::addConnection( const QgsSensorThingsConnection &conn )
{
  QgsSensorThingsConnection::settingsUrl->setValue( conn.url, conn.name );
  QgsSensorThingsConnection::settingsAuthcfg->setValue( conn.authCfg, conn.name );
  QgsSensorThingsConnection::settingsUsername->setValue( conn.username, conn.name );
  QgsSensorThingsConnection::settingsPassword->setValue( conn.password, conn.name );
  QgsSensorThingsConnection::settingsHeaders->setValue( conn.httpHeaders.headers(), conn.name );
}

