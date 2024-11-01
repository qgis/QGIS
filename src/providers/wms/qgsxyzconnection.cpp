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

#include <qgslogger.h>
#include "qgsxyzconnection.h"

#include "qgsowsconnection.h"
#include "qgsdatasourceuri.h"
#include "qgssettingsentryimpl.h"


QString QgsXyzConnection::encodedUri() const
{
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uri.setParam( QStringLiteral( "url" ), url );
  if ( zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( zMin ) );
  if ( zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( zMax ) );
  if ( !authCfg.isEmpty() )
    uri.setAuthConfigId( authCfg );
  if ( !username.isEmpty() )
    uri.setUsername( username );
  if ( !password.isEmpty() )
    uri.setPassword( password );

  uri.setHttpHeaders( httpHeaders );

  if ( tilePixelRatio != 0 )
    uri.setParam( QStringLiteral( "tilePixelRatio" ), QString::number( tilePixelRatio ) );
  if ( !interpretation.isEmpty() )
    uri.setParam( QStringLiteral( "interpretation" ), interpretation );
  return uri.encodedUri();
}

QStringList QgsXyzConnectionUtils::connectionList()
{
  QStringList list = QgsXyzConnectionSettings::sTreeXyzConnections->items();
  for ( const QString &connection : std::as_const( list ) )
  {
    if ( QgsXyzConnectionSettings::settingsUrl->origin( { connection } ) == Qgis::SettingsOrigin::Global )
      if ( QgsXyzConnectionSettings::settingsHidden->value( connection ) )
        list.removeOne( connection );
  }
  return list;
}

QgsXyzConnection QgsXyzConnectionUtils::connection( const QString &name )
{
  QgsXyzConnection conn;
  conn.name = name;
  conn.url = QgsXyzConnectionSettings::settingsUrl->value( name );
  conn.zMin = QgsXyzConnectionSettings::settingsZmin->value( name );
  conn.zMax = QgsXyzConnectionSettings::settingsZmax->value( name );
  conn.authCfg = QgsXyzConnectionSettings::settingsAuthcfg->value( name );
  conn.username = QgsXyzConnectionSettings::settingsUsername->value( name );
  conn.password = QgsXyzConnectionSettings::settingsPassword->value( name );
  conn.httpHeaders = QgsXyzConnectionSettings::settingsHeaders->value( name );
  conn.tilePixelRatio = static_cast<int>( QgsXyzConnectionSettings::settingsTilePixelRatio->value( name ) );
  conn.hidden = QgsXyzConnectionSettings::settingsHidden->value( name );
  conn.interpretation = QgsXyzConnectionSettings::settingsInterpretation->value( name );
  return conn;
}

void QgsXyzConnectionUtils::deleteConnection( const QString &name )
{
  if ( QgsXyzConnectionSettings::settingsUrl->origin( { name } ) == Qgis::SettingsOrigin::Global )
  {
    QgsXyzConnectionSettings::settingsHidden->setValue( true, name );
  }
  else
  {
    QgsXyzConnectionSettings::sTreeXyzConnections->deleteItem( name );
  }
}

void QgsXyzConnectionUtils::addConnection( const QgsXyzConnection &conn )
{
  QgsXyzConnectionSettings::settingsUrl->setValue( conn.url, conn.name );
  QgsXyzConnectionSettings::settingsZmin->setValue( conn.zMin, conn.name );
  QgsXyzConnectionSettings::settingsZmax->setValue( conn.zMax, conn.name );
  QgsXyzConnectionSettings::settingsAuthcfg->setValue( conn.authCfg, conn.name );
  QgsXyzConnectionSettings::settingsUsername->setValue( conn.username, conn.name );
  QgsXyzConnectionSettings::settingsPassword->setValue( conn.password, conn.name );
  QgsXyzConnectionSettings::settingsHeaders->setValue( conn.httpHeaders.headers(), conn.name );
  QgsXyzConnectionSettings::settingsTilePixelRatio->setValue( conn.tilePixelRatio, conn.name );
  QgsXyzConnectionSettings::settingsInterpretation->setValue( conn.interpretation, conn.name );

  if ( QgsXyzConnectionSettings::settingsUrl->origin( { conn.name } ) == Qgis::SettingsOrigin::Global )
    QgsXyzConnectionSettings::settingsHidden->setValue( false, conn.name );
}
