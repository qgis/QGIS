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
    Capability::DropVectorTable,
    Capability::CreateVectorTable,
    Capability::RenameSchema,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::RenameTable,
    Capability::Vacuum,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    //Capability::Transaction,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists
  };
}

void QgsPostgresProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ) );
}

void QgsPostgresProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString,
    QVariant> *options ) const
{

  checkCapability( Capability::CreateVectorTable );

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

void QgsPostgresProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  dropTablePrivate( schema, name );
}

void QgsPostgresProviderConnection::dropRasterTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropRasterTable );
  dropTablePrivate( schema, name );
}

void QgsPostgresProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameTable );
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ) );
}

void QgsPostgresProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) ) );

}

void QgsPostgresProviderConnection::dropSchema( const QString &name,  bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1 %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlPrivate( QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ) );
}

QList<QVariantList> QgsPostgresProviderConnection::executeSql( const QString &sql ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql );
}

QList<QVariantList> QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql ) const
{
  const QgsDataSourceUri dsUri { uri() };
  QList<QVariantList> results;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( dsUri.connectionInfo( false ) );
  if ( !conn )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }
  else
  {
    QgsPostgresResult res( conn->PQexec( sql ) );
    QString errCause;
    if ( conn->PQstatus() != CONNECTION_OK || ! res.result() )
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
    for ( int rowIdx = 0; rowIdx < res.PQntuples(); rowIdx++ )
    {
      QVariantList row;
      for ( int colIdx = 0; colIdx < res.PQnfields(); colIdx++ )
      {
        row.push_back( res.PQgetvalue( rowIdx, colIdx ) );
      }
      results.push_back( row );
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( ! errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }
  }
  return results;
}

void QgsPostgresProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::Vacuum );
  executeSql( QStringLiteral( "VACUUM FULL ANALYZE %1.%2" )
              .arg( QgsPostgresConn::quotedIdentifier( schema ) )
              .arg( QgsPostgresConn::quotedIdentifier( name ) ) );
}

QList<QgsPostgresProviderConnection::TableProperty> QgsPostgresProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsPostgresProviderConnection::TableProperty> tables;
  QString errCause;
  // TODO: set flags from the connection if flags argument is 0
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
      const bool aspatial { ! flags || flags.testFlag( TableFlag::Aspatial ) };
      conn->supportedLayers( properties, false, schema == QStringLiteral( "public" ), aspatial, schema );
      bool dontResolveType = QgsPostgresConn::dontResolveType( name() );

      // Cannot be const:
      for ( auto &pr : properties )
      {
        // Classify
        TableFlags prFlags;
        if ( pr.isView )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::View );
        }
        if ( pr.isMaterializedView )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::MaterializedView );
        }
        // Table type
        if ( pr.isRaster )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::Raster );
        }
        else if ( pr.nSpCols != 0 )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::Vector );
        }
        else
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::Aspatial );
        }
        // Filter
        if ( ! flags || ( prFlags & flags ) )
        {
          // retrieve layer types if needed
          if ( ! dontResolveType && ( !pr.geometryColName.isNull() &&
                                      ( pr.types.value( 0, QgsWkbTypes::Unknown ) == QgsWkbTypes::Unknown ||
                                        pr.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) ) )
          {
            conn->retrieveLayerTypes( pr, true /* useEstimatedMetadata */ );
          }
          QgsPostgresProviderConnection::TableProperty property;
          property.setFlags( prFlags );
          for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ) ; i++ )
          {
            property.addGeometryType( pr.types.at( i ), pr.srids.at( i ) );
          }
          property.setTableName( pr.tableName );
          property.setSchema( pr.schemaName );
          property.setGeometryColumn( pr.geometryColName );
          property.setPkColumns( pr.pkCols );
          property.setGeometryColumnCount( static_cast<int>( pr.nSpCols ) );
          property.setSql( pr.sql );
          property.setComment( pr.tableComment );
          tables.push_back( property );
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

QStringList QgsPostgresProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
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


void QgsPostgresProviderConnection::store( QVariantMap guiConfig ) const
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

void QgsPostgresProviderConnection::remove() const
{
  QgsPostgresConn::deleteConnection( name() );
}

