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
#include "qgssettings.h"
#include "qgspostgresprovider.h"

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setUri( QgsPostgresConn::connUri( name ) );
  setDefaultCapabilities();
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name, const QgsDataSourceUri &uri ):
  QgsAbstractDatabaseProviderConnection( name, uri )
{
  setUri( uri );
  setDefaultCapabilities();
}



void QgsPostgresProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropTable,
    Capability::CreateVectorTable,
    // Capability::CreateRasterTable,
    Capability::RenameSchema,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::RenameTable,
    Capability::Vacuum,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    //Capability::Transaction,
    Capability::Tables,
    Capability::Schemas
  };
}



bool QgsPostgresProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString,
    QVariant> *options,
    QString &errCause )
{

  if ( capabilities().testFlag( Capability::CreateVectorTable ) )
  {
    auto newUri { uri() };
    newUri.setSchema( schema );
    newUri.setTable( name );
    QMap<int, int> map;
    QgsVectorLayerExporter::ExportError errCode = QgsPostgresProvider::createEmptyLayer(
          newUri.uri(),
          fields,
          wkbType,
          srs,
          overwrite,
          &map,
          &errCause,
          options
        );
    return errCode == QgsVectorLayerExporter::ExportError::NoError;
  }
  return false;
}

bool QgsPostgresProviderConnection::dropTable( const QString &schema, const QString &name, QString &errCause )
{
  return capabilities().testFlag( Capability::DropTable ) &&
         executeSql( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ), errCause );
}

bool QgsPostgresProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName, QString &errCause )
{
  return capabilities().testFlag( Capability::RenameTable ) &&
         executeSql( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ), errCause );
}

bool QgsPostgresProviderConnection::createSchema( const QString &name, QString &errCause )
{
  return capabilities().testFlag( Capability::CreateSchema ) &&
         executeSql( QStringLiteral( "CREATE SCHEMA %1" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ), errCause );

}

bool QgsPostgresProviderConnection::dropSchema( const QString &name, QString &errCause )
{
  return capabilities().testFlag( Capability::DropSchema ) &&
         executeSql( QStringLiteral( "DROP SCHEMA %1" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ), errCause );
}

bool QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName, QString &errCause )
{
  return capabilities().testFlag( Capability::RenameSchema ) &&
         executeSql( QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ), errCause );
}

bool QgsPostgresProviderConnection::executeSql( const QString &sql, QString &errCause )
{
  return capabilities().testFlag( Capability::ExecuteSql ) &&
         executeSqlPrivate( sql, errCause );
}

bool QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql, QString &errCause )
{
  bool ok = false;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri().connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri().uri() );
  }
  else
  {
    ok = conn->PQexecNR( sql );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not execute SQL %1: %2" ).arg( sql ).arg( conn->PQerrorMessage() );
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
  }
  return  ok;
}

bool QgsPostgresProviderConnection::vacuum( const QString &schema, const QString &name,  QString &errCause )
{
  return capabilities().testFlag( Capability::Vacuum ) &&
         executeSql( QStringLiteral( "VACUUM FULL ANALYZE %1.%2" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ), errCause );
}

QStringList QgsPostgresProviderConnection::tables( const QString &schema, QString &errCause )
{

  QStringList tables;
  if ( capabilities().testFlag( Capability::Tables ) )
  {
    QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri().connectionInfo( false ) );
    if ( !conn )
    {
      errCause = QObject::tr( "Connection failed: %1" ).arg( uri().uri() );
    }
    else
    {
      bool ok = conn->getTableInfo( false, false, true, schema );
      if ( ! ok )
      {
        errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri().uri() );
      }
      else
      {
        QVector<QgsPostgresLayerProperty> properties;
        conn->supportedLayers( properties, false, false, true, schema );
        for ( const auto &p : qgis::as_const( properties ) )
        {
          tables.push_back( p.tableName );
        }
      }
      QgsPostgresConnPool::instance()->releaseConnection( conn );
    }
  }
  return tables;
}

QStringList QgsPostgresProviderConnection::schemas( QString &errCause )
{
  QStringList schemas;
  if ( capabilities().testFlag( Capability::Schemas ) )
  {
    QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri().connectionInfo( false ) );
    if ( !conn )
    {
      errCause = QObject::tr( "Connection failed: %1" ).arg( uri().uri() );
    }
    else
    {
      QList<QgsPostgresSchemaProperty> schemaProperties;
      bool ok = conn->getSchemas( schemaProperties );
      QgsPostgresConnPool::instance()->releaseConnection( conn );
      if ( ! ok )
      {
        errCause = QObject::tr( "Could not retrieve schemas: %1" ).arg( uri().uri() );
      }
      else
      {
        for ( const auto &s : qgis::as_const( schemaProperties ) )
        {
          schemas.push_back( s.name );
        }
      }
    }
  }
  return schemas;
}


bool QgsPostgresProviderConnection::store( QVariantMap guiConfig )
{
  // TODO: move this to class configuration?
  QString baseKey = QStringLiteral( "/PostgreSQL/connections/" );
  // delete the original entry first
  remove( );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( connectionName() );

  // From URI
  settings.setValue( "service", uri().service() );
  settings.setValue( "host",  uri().host() );
  settings.setValue( "port", uri().port() );
  settings.setValue( "database", uri().database() );
  settings.setValue( "username", uri().username() );
  settings.setValue( "password", uri().password() );
  settings.setValue( "authcfg", uri().authConfigId() );
  settings.setValue( "sslmode", uri().sslMode() );

  // From GUI config
  static const QStringList guiParameters
  {
    QStringLiteral( "publicOnly" ),
    QStringLiteral( "geometryColumnsOnly" ),
    QStringLiteral( "dontResolveType" ),
    QStringLiteral( "allowGeometrylessTables" ),
    QStringLiteral( "saveUsername" ),
    QStringLiteral( "savePassword" ),
    QStringLiteral( "estimatedMetadata" ),
    QStringLiteral( "projectsInDatabase" )
  };
  for ( const auto &p : guiParameters )
  {
    if ( guiConfig.contains( p ) )
    {
      settings.setValue( p, guiConfig.value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
  return true;
}

bool QgsPostgresProviderConnection::remove()
{
  QgsPostgresConn::deleteConnection( connectionName() );
  return true;
}

