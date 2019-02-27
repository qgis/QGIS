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

#include "qgsdatasourceuri.h"
#include "qgssettings.h"

QString QgsXyzConnection::encodedUri() const
{
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uri.setParam( QStringLiteral( "url" ), url );
  if ( zMin != -1 )
    uri.setParam( QStringLiteral( "zmin" ), QString::number( zMin ) );
  if ( zMax != -1 )
    uri.setParam( QStringLiteral( "zmax" ), QString::number( zMax ) );
  if ( ! authCfg.isEmpty() )
    uri.setParam( QStringLiteral( "authcfg" ), authCfg );
  if ( ! username.isEmpty() )
    uri.setParam( QStringLiteral( "username" ), username );
  if ( ! password.isEmpty() )
    uri.setParam( QStringLiteral( "password" ), password );
  if ( ! referer.isEmpty() )
    uri.setParam( QStringLiteral( "referer" ), referer );
  if ( tilePixelRatio != 0 )
    uri.setParam( QStringLiteral( "tilePixelRatio" ), QString::number( tilePixelRatio ) );
  return uri.encodedUri();
}

QStringList QgsXyzConnectionUtils::connectionList()
{
  QgsSettings settings;
  QStringList connList;

  settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
  connList = settings.childGroups();

  const QStringList global = settings.globalChildGroups();
  settings.endGroup();

  for ( const auto &s : global )
  {
    settings.beginGroup( "qgis/connections-xyz/" + s );
    bool isHidden = settings.value( QStringLiteral( "hidden" ), false ).toBool();
    settings.endGroup();
    if ( isHidden )
    {
      connList.removeOne( s );
    }
  }

  return connList;
}

QgsXyzConnection QgsXyzConnectionUtils::connection( const QString &name )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-xyz/" + name );

  QgsXyzConnection conn;
  conn.name = name;
  conn.url = settings.value( QStringLiteral( "url" ) ).toString();
  conn.zMin = settings.value( QStringLiteral( "zmin" ), -1 ).toInt();
  conn.zMax = settings.value( QStringLiteral( "zmax" ), -1 ).toInt();
  conn.authCfg = settings.value( QStringLiteral( "authcfg" ) ).toString();
  conn.username = settings.value( QStringLiteral( "username" ) ).toString();
  conn.password = settings.value( QStringLiteral( "password" ) ).toString();
  conn.referer = settings.value( QStringLiteral( "referer" ) ).toString();
  conn.tilePixelRatio = settings.value( QStringLiteral( "tilePixelRatio" ), 0 ).toDouble();
  conn.hidden = settings.value( QStringLiteral( "hidden" ) ).toBool();
  return conn;
}

void QgsXyzConnectionUtils::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-xyz/" + name );

  settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
  QStringList global = settings.globalChildGroups();

  if ( global.contains( name ) )
  {
    QgsSettings settings;
    settings.beginGroup( "qgis/connections-xyz/" + name );
    settings.setValue( QStringLiteral( "hidden" ), true );
  }

}

void QgsXyzConnectionUtils::addConnection( const QgsXyzConnection &conn )
{
  QgsSettings settings;
  bool addHiddenProperty = false;

  settings.beginGroup( QStringLiteral( "qgis/connections-xyz" ) );
  QStringList global = settings.globalChildGroups();
  if ( global.contains( conn.name ) )
  {
    addHiddenProperty = true;
  }
  settings.endGroup();

  settings.beginGroup( "qgis/connections-xyz/" + conn.name );
  settings.setValue( QStringLiteral( "url" ), conn.url );
  settings.setValue( QStringLiteral( "zmin" ), conn.zMin );
  settings.setValue( QStringLiteral( "zmax" ), conn.zMax );
  settings.setValue( QStringLiteral( "authcfg" ), conn.authCfg );
  settings.setValue( QStringLiteral( "username" ), conn.username );
  settings.setValue( QStringLiteral( "password" ), conn.password );
  settings.setValue( QStringLiteral( "referer" ), conn.referer );
  settings.setValue( QStringLiteral( "tilePixelRatio" ), conn.tilePixelRatio );
  if ( addHiddenProperty )
  {
    settings.setValue( QStringLiteral( "hidden" ), false );
  }

}
