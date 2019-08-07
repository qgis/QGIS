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
#include "qgsexception.h"


extern "C"
{
#include <libpq-fe.h>
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setUri( QgsPostgresConn::connUri( name ).uri() );
  setDefaultCapabilities();
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name, const QString &uri ):
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



void QgsPostgresProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString,
    QVariant> *options )
{

  if ( capabilities().testFlag( Capability::CreateVectorTable ) )
  {
    QgsDataSourceUri newUri { uri() };
    newUri.setSchema( schema );
    newUri.setTable( name );
    // Set geometry column if it's not aspatial
    if ( wkbType != QgsWkbTypes::Type::Unknown &&  wkbType != QgsWkbTypes::Type::NoGeometry )
    {
      newUri.setGeometryColumn( options->value( QStringLiteral( "geometryColumn" ), QStringLiteral( "geom" ) ).toString() );
    }
    QMap<int, int> map;
    QString errCause;
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
    if ( errCode != QgsVectorLayerExporter::ExportError::NoError )
    {
      throw QgsProviderConnectionException( QObject::tr( "An error occourred while creating the vector layer: %1" ).arg( errCause ) );
    }
  }
}

void QgsPostgresProviderConnection::dropTable( const QString &schema, const QString &name )
{
  if ( ! capabilities().testFlag( Capability::DropTable ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ) );
}

void QgsPostgresProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName )
{
  if ( ! capabilities().testFlag( Capability::RenameTable ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ) );
}

void QgsPostgresProviderConnection::createSchema( const QString &name )
{
  if ( ! capabilities().testFlag( Capability::CreateSchema ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ) );

}

void QgsPostgresProviderConnection::dropSchema( const QString &name,  bool force )
{
  if ( ! capabilities().testFlag( Capability::DropSchema ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1 %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName )
{
  if ( ! capabilities().testFlag( Capability::RenameSchema ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ) );
}

void QgsPostgresProviderConnection::executeSql( const QString &sql )
{
  if ( ! capabilities().testFlag( Capability::ExecuteSql ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSqlPrivate( sql );
}

void QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql )
{
  const QgsDataSourceUri dsUri { uri() };
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }
  else
  {
    PGresult *res = conn->PQexec( sql );
    QString errCause;
    if ( conn->PQstatus() != CONNECTION_OK || ! res )
    {
      errCause = QObject::tr( "Connection error: %1 returned %2 [%3]" )
                 .arg( sql ).arg( conn->PQstatus() )
                 .arg( conn->PQerrorMessage() );
    }
    else
    {
      const QString err { conn->PQerrorMessage() };
      if ( ! err.isEmpty() )
      {
        errCause = QObject::tr( "SQL error: %1 returned %2 [%3]" )
                   .arg( sql )
                   .arg( conn->PQstatus() )
                   .arg( err );
      }
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( ! errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }
  }
}

void QgsPostgresProviderConnection::vacuum( const QString &schema, const QString &name )
{
  if ( ! capabilities().testFlag( Capability::Vacuum ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  executeSql( QStringLiteral( "VACUUM FULL ANALYZE %1.%2" )
              .arg( QgsPostgresConn::quotedIdentifier( schema ) )
              .arg( QgsPostgresConn::quotedIdentifier( name ) ) );
}

QList<QgsPostgresProviderConnection::TableProperty> QgsPostgresProviderConnection::tables( const QString &schema, const TableFlags &flags )
{
  if ( ! capabilities().testFlag( Capability::Tables ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }

  QList<QgsPostgresProviderConnection::TableProperty> tables;
  QString errCause;
  const QgsDataSourceUri dsUri { uri() };
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    bool ok = conn->getTableInfo( false, false, true, schema );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri() );
    }
    else
    {
      QVector<QgsPostgresLayerProperty> properties;
      const bool aspatial { flags == TableFlag::None || flags.testFlag( TableFlag::Aspatial ) };
      conn->supportedLayers( properties, false, schema == QStringLiteral( "public" ), aspatial, schema );

      // Utility to create a TableProperty and insert in the result list
      auto fetch_property = [ &flags, &tables ]( const QgsPostgresLayerProperty & p )
      {
        QgsPostgresProviderConnection::TableProperty property;
        if ( p.isView )
        {
          property.flags.setFlag( QgsPostgresProviderConnection::TableFlag::View );
        }
        if ( p.isMaterializedView )
        {
          property.flags.setFlag( QgsPostgresProviderConnection::TableFlag::MaterializedView );
        }
        if ( p.isRaster )
        {
          property.flags.setFlag( QgsPostgresProviderConnection::TableFlag::Raster );
        }
        else
        {
          property.flags.setFlag( QgsPostgresProviderConnection::TableFlag::Vector );
        }
        if ( ! p.isRaster && p.nSpCols == 0 )
        {
          property.flags.setFlag( QgsPostgresProviderConnection::TableFlag::Aspatial );
        }
        // Filters
        if ( flags == TableFlag::None || ( property.flags ^ flags ) )
        {
          property.types = p.types;
          property.name = p.tableName;
          property.schema = p.schemaName;
          property.geometryColumn = p.geometryColName;
          property.pkColumns = p.pkCols;
          property.srids = p.srids;
          property.spatialColumnCount = p.nSpCols;
          property.sql = p.sql;
          property.tableComment = p.tableComment;
          tables.push_back( property );
        }
      };

      for ( const auto &pr : qgis::as_const( properties ) )
      {
        // Aspatial
        if ( pr.size() == 0 )
        {
          fetch_property( pr );
        }
        // Handle multiple geometry columns
        for ( unsigned int i = 0; i < pr.nSpCols; i++ )
        {
          const auto prAt { pr.at( i ) };
          fetch_property( prAt );
        }
      }
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return tables;
}

QStringList QgsPostgresProviderConnection::schemas( )
{
  if ( ! capabilities().testFlag( Capability::Schemas ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }

  QStringList schemas;
  QString errCause;
  const QgsDataSourceUri dsUri { uri() };
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    QList<QgsPostgresSchemaProperty> schemaProperties;
    bool ok = conn->getSchemas( schemaProperties );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not retrieve schemas: %1" ).arg( uri() );
    }
    else
    {
      for ( const auto &s : qgis::as_const( schemaProperties ) )
      {
        schemas.push_back( s.name );
      }
    }
  }
  if ( ! errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return schemas;
}


void QgsPostgresProviderConnection::store( QVariantMap guiConfig )
{
  // TODO: move this to class configuration?
  QString baseKey = QStringLiteral( "/PostgreSQL/connections/" );
  // delete the original entry first
  remove( );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name() );

  // From URI
  const QgsDataSourceUri dsUri { uri() };
  settings.setValue( "service", dsUri.service() );
  settings.setValue( "host",  dsUri.host() );
  settings.setValue( "port", dsUri.port() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "authcfg", dsUri.authConfigId() );
  settings.setValue( "sslmode", dsUri.sslMode() );

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
}

void QgsPostgresProviderConnection::remove()
{
  QgsPostgresConn::deleteConnection( name() );
}

