/***************************************************************************
    qgsgeonodeconnection.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettings.h"
#include "qgsgeonodeconnection.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"

const QString QgsGeoNodeConnectionUtils::sGeoNodeConnection = QStringLiteral( "GEONODE" );

QgsGeoNodeConnection::QgsGeoNodeConnection( const QString &name )
  : mConnName( name )
{
  mUri.setParam( QStringLiteral( "url" ), QgsOwsConnection::settingsConnectionUrl.value( {QgsGeoNodeConnectionUtils::sGeoNodeConnection.toLower(), mConnName} ) );

  // Check for credentials and prepend to the connection info
  const QString username = QgsOwsConnection::settingsConnectionUsername.value( {QgsGeoNodeConnectionUtils::sGeoNodeConnection, mConnName} );
  const QString password = QgsOwsConnection::settingsConnectionPassword.value( {QgsGeoNodeConnectionUtils::sGeoNodeConnection, mConnName} );
  if ( !username.isEmpty() )
  {
    mUri.setUsername( username );
    mUri.setPassword( password );
  }

  const QString authcfg = QgsOwsConnection::settingsConnectionAuthCfg.value( {QgsGeoNodeConnectionUtils::sGeoNodeConnection, mConnName} );
  if ( !authcfg.isEmpty() )
  {
    mUri.setAuthConfigId( authcfg );
  }

  QgsDebugMsgLevel( QStringLiteral( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ), 4 );
}

QgsDataSourceUri QgsGeoNodeConnection::uri() const
{
  return mUri;
}

QString QgsGeoNodeConnection::connectionName() const
{
  return mConnName;
}

void QgsGeoNodeConnection::setConnectionName( const QString &connName )
{
  mConnName = connName;
}

void QgsGeoNodeConnection::setUri( const QgsDataSourceUri &uri )
{
  mUri = uri;
}

QgsDataSourceUri &QgsGeoNodeConnection::addWmsConnectionSettings( QgsDataSourceUri &uri ) const
{
  QString detailedConnectionName = QStringLiteral( "%1/wms" ).arg( mConnName );
  return QgsOwsConnection::addWmsWcsConnectionSettings( uri, QgsGeoNodeConnectionUtils::sGeoNodeConnection, detailedConnectionName );
}

QgsDataSourceUri &QgsGeoNodeConnection::addWfsConnectionSettings( QgsDataSourceUri &uri ) const
{
  QString detailedConnectionName = QStringLiteral( "%1/wfs" ).arg( mConnName );
  return QgsOwsConnection::addWfsConnectionSettings( uri, QgsGeoNodeConnectionUtils::sGeoNodeConnection, detailedConnectionName );
}

QgsDataSourceUri &QgsGeoNodeConnection::addWcsConnectionSettings( QgsDataSourceUri &uri ) const
{
  QString detailedConnectionName = QStringLiteral( "%1/wcs" ).arg( mConnName );
  return QgsOwsConnection::addWmsWcsConnectionSettings( uri, QgsGeoNodeConnectionUtils::sGeoNodeConnection, detailedConnectionName );
}

//
// QgsGeoNodeConnectionUtils
//


QStringList QgsGeoNodeConnectionUtils::connectionList()
{
  QgsSettings settings;
  // Add Section manually
  settings.beginGroup( QStringLiteral( "qgis/connections-geonode" ) );
  return settings.childGroups();
}

void QgsGeoNodeConnectionUtils::deleteConnection( const QString &name )
{
  QgsOwsConnection::deleteConnection( sGeoNodeConnection, name );
}

QString QgsGeoNodeConnectionUtils::pathGeoNodeConnection()
{
  return QStringLiteral( "qgis/connections-%1" ).arg( sGeoNodeConnection.toLower() );
}

QString QgsGeoNodeConnectionUtils::pathGeoNodeConnectionDetails()
{
  return QStringLiteral( "qgis/%1" ).arg( sGeoNodeConnection );
}
