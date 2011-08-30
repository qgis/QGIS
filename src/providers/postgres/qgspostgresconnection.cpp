/***************************************************************************
    qgspostgresconnection.cpp  -  PostgresSQL/PostGIS connection
                             -------------------
    begin                : 3 June 2011
    copyright            : (C) 2011 by Giuseppe Sucameli
    email                : brush.tyler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresconnection.h"

#include <qgslogger.h>
#include "qgspostgresprovider.h"
#include "qgsproviderregistry.h"
#include "qgsdatasourceuri.h"

#include <QSettings>

QStringList QgsPostgresConnection::connectionList()
{
  QSettings settings;
  settings.beginGroup( "/PostgreSQL/connections" );
  return settings.childGroups();
}

QString QgsPostgresConnection::selectedConnection()
{
  QSettings settings;
  return settings.value( "/PostgreSQL/connections/selected" ).toString();
}

void QgsPostgresConnection::setSelectedConnection( QString name )
{
  QSettings settings;
  return settings.setValue( "/PostgreSQL/connections/selected", name );
}


QgsPostgresConnection::QgsPostgresConnection( QString theConnName ) :
    mConnName( theConnName )
{
  QgsDebugMsg( "theConnName = " + theConnName );

  QSettings settings;

  QString key = "/PostgreSQL/connections/" + mConnName;

  QString service = settings.value( key + "/service" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = "5432";
  }
  QString database = settings.value( key + "/database" ).toString();

  //bool publicSchemaOnly = settings.value( key + "/publicOnly", false ).toBool();
  //bool geometryColumnsOnly = settings.value( key + "/geometrycolumnsOnly", false ).toBool();
  //bool allowGeometrylessTables = settings.value( key + "/allowGeometrylessTables", false ).toBool();

  bool useEstimatedMetadata = settings.value( key + "/estimatedMetadata", false ).toBool();
  int sslmode = settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt();

  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == "true" )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true" )
  {
    password = settings.value( key + "/password" ).toString();
  }

  // Old save setting
  if ( settings.contains( key + "/save" ) )
  {
    username = settings.value( key + "/username" ).toString();

    if ( settings.value( key + "/save" ).toString() == "true" )
    {
      password = settings.value( key + "/password" ).toString();
    }
  }

  QgsDataSourceURI uri;
  if ( !service.isEmpty() )
  {
    uri.setConnection( service, database, username, password, ( QgsDataSourceURI::SSLmode ) sslmode );
  }
  else
  {
    uri.setConnection( host, port, database, username, password, ( QgsDataSourceURI::SSLmode ) sslmode );
  }
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  mConnectionInfo = uri.connectionInfo();

  QgsDebugMsg( QString( "Connection info: '%1'." ).arg( mConnectionInfo ) );
}

QgsPostgresConnection::~QgsPostgresConnection()
{

}

QString QgsPostgresConnection::connectionInfo( )
{
  return mConnectionInfo;
}

QgsPostgresProvider * QgsPostgresConnection::provider( )
{
  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  QgsPostgresProvider *postgresProvider =
    ( QgsPostgresProvider* ) pReg->provider( "postgres", mConnectionInfo );

  return postgresProvider;
}

