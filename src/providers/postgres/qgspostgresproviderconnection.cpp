/***************************************************************************
  qgspostgresproviderconnection.cpp - QgsPostgresProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgresproviderconnection.h"
#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QgsDataSourceUri &uri ):
  mUri( uri )
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
    Capability::DropTable |
    Capability::CreateTable |
    Capability::RenameSchema |
    Capability::DropSchema |
    Capability::CreateSchema |
    Capability::RenameTable |
    Capability::Vacuum |
    Capability::ExecuteSQL |
    //Capability::Transaction |
    Capability::Tables |
    Capability::Schemas;
}


bool QgsPostgresProviderConnection::createTable( const QString &name, const QString &schema, QString &errCause )
{
}

bool QgsPostgresProviderConnection::dropTable( const QString &name, const QString &schema, QString &errCause )
{
}

bool QgsPostgresProviderConnection::renameTable( const QString &name, const QString &schema, const QString &newName, QString &errCause )
{
}

bool QgsPostgresProviderConnection::createSchema( const QString &name, QString &errCause )
{
}

bool QgsPostgresProviderConnection::dropSchema( const QString &name, QString &errCause )
{
}

bool QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName, QString &errCause )
{
}

QVariant QgsPostgresProviderConnection::executeSql( const QString &sql, QString &errCause )
{
}

bool QgsPostgresProviderConnection::vacuum( const QString &name, QString &errCause )
{
  bool ok = false;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( mUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( mUri.uri() );
  }
  else
  {
    ok = conn->PQexecNR( QStringLiteral( "VACUUM FULL ANALIZE %s" ).arg( QgsPostgresConn::quotedValue( name ) ) );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not execute 'vacuum' on table '%1': %2" ).arg( name ).arg( mUri.uri() );
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
  }
  return  ok;
}

QStringList QgsPostgresProviderConnection::tables( const QString &schema, QString &errCause )
{
  QStringList tables;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( mUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( mUri.uri() );
  }
  else
  {
    QList<QgsPostgresSchemaProperty> schemaProperties;
    bool ok = conn->getTableInfo( false, false, false, schema );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( mUri.uri() );
    }
    else
    {
      for ( const auto &s : qgis::as_const( schemaProperties ) )
      {
        tables.push_back( s.name );
      }
    }
  }
  return tables;
}

QStringList QgsPostgresProviderConnection::schemas( QString &errCause )
{
  QStringList schemas;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( mUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( mUri.uri() );
  }
  else
  {
    QList<QgsPostgresSchemaProperty> schemaProperties;
    bool ok = conn->getSchemas( schemaProperties );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not retrieve schemas: %1" ).arg( mUri.uri() );
    }
    else
    {
      for ( const auto &s : qgis::as_const( schemaProperties ) )
      {
        schemas.push_back( s.name );
      }
    }
  }
  return schemas;
}
