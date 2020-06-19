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
#include "qgsapplication.h"

extern "C"
{
#include <libpq-fe.h>
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
  setUri( QgsPostgresConn::connUri( name ).uri().replace( removePartsRe, QString() ) );
  setDefaultCapabilities();
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  setDefaultCapabilities();
}

void QgsPostgresProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropVectorTable,
    Capability::DropRasterTable,
    Capability::CreateVectorTable,
    Capability::RenameSchema,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::RenameVectorTable,
    Capability::RenameRasterTable,
    Capability::Vacuum,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    //Capability::Transaction,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists,
    Capability::CreateSpatialIndex,
    Capability::SpatialIndexExists,
    Capability::DeleteSpatialIndex
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
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsPostgresProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  return dsUri.uri( false );
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


void QgsPostgresProviderConnection::renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const
{
  executeSqlPrivate( QStringLiteral( "ALTER TABLE %1.%2 RENAME TO %3" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ) )
                     .arg( QgsPostgresConn::quotedIdentifier( name ) )
                     .arg( QgsPostgresConn::quotedIdentifier( newName ) ) );
}

void QgsPostgresProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTablePrivate( schema, name, newName );
}

void QgsPostgresProviderConnection::renameRasterTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameRasterTable );
  renameTablePrivate( schema, name, newName );
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

QList<QVariantList> QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes ) const
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
    if ( res.PQntuples() > 0 )
    {
      // Try to convert value types at least for basic simple types that can be directly mapped to Python
      QMap<int, QVariant::Type> typeMap;
      if ( resolveTypes )
      {
        for ( int rowIdx = 0; rowIdx < res.PQnfields(); rowIdx++ )
        {
          const Oid oid { res.PQftype( rowIdx ) };
          QList<QVariantList> typeRes { executeSqlPrivate( QStringLiteral( "SELECT typname FROM pg_type WHERE oid = %1" ).arg( oid ), false ) };
          // Set the default to string
          QVariant::Type vType { QVariant::Type::String };
          if ( typeRes.size() > 0 && typeRes.first().size() > 0 )
          {
            static const QStringList intTypes = { QStringLiteral( "oid" ),
                                                  QStringLiteral( "int2" ),
                                                  QStringLiteral( "int4" ),
                                                  QStringLiteral( "int8" )
                                                };
            static const QStringList floatTypes = { QStringLiteral( "float4" ),
                                                    QStringLiteral( "float8" ),
                                                    QStringLiteral( "numeric" )
                                                  };
            const QString typName { typeRes.first().first().toString() };

            if ( floatTypes.contains( typName ) )
            {
              vType = QVariant::Double;
            }
            else if ( intTypes.contains( typName ) )
            {
              vType = QVariant::LongLong;
            }
            else if ( typName == QStringLiteral( "date" ) )
            {
              vType = QVariant::Date;
            }
            else if ( typName.startsWith( QStringLiteral( "timestamp" ) ) )
            {
              vType = QVariant::DateTime;
            }
            else if ( typName == QStringLiteral( "time" ) )
            {
              vType = QVariant::Time;
            }
            else if ( typName == QStringLiteral( "bool" ) )
            {
              vType = QVariant::Bool;
            }
            else if ( typName == QStringLiteral( "char" ) )
            {
              vType = QVariant::Char;
            }
            else
            {
              // Just a warning, usually ok
              QgsDebugMsgLevel( QStringLiteral( "Unhandled PostgreSQL type %1, assuming string" ).arg( typName ), 2 );
            }
          }
          typeMap[ rowIdx ] = vType;
        }
      }
      for ( int rowIdx = 0; rowIdx < res.PQntuples(); rowIdx++ )
      {
        QVariantList row;
        for ( int colIdx = 0; colIdx < res.PQnfields(); colIdx++ )
        {
          if ( resolveTypes )
          {
            const QVariant::Type vType { typeMap.value( colIdx, QVariant::Type::String ) };
            QVariant val { res.PQgetvalue( rowIdx, colIdx ) };
            // Special case for bools: 'f' and 't'
            if ( vType == QVariant::Bool )
            {
              if ( ! val.toString().isEmpty() )
              {
                val = val.toString() == 't';
              }
            }
            else if ( val.canConvert( static_cast<int>( vType ) ) )
            {
              val.convert( static_cast<int>( vType ) );
            }
            row.push_back( val );
          }
          else
          {
            row.push_back( res.PQgetvalue( rowIdx, colIdx ) );
          }
        }
        results.push_back( row );
      }
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

void QgsPostgresProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  if ( options.geometryColumnName.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Geometry column name not specified while creating spatial index" ) );

  checkCapability( Capability::CreateSpatialIndex );

  const QString indexName = QStringLiteral( "sidx_%1_%2" ).arg( name, options.geometryColumnName );
  executeSql( QStringLiteral( "CREATE INDEX %1 ON %2.%3 USING GIST (%4);" )
              .arg( indexName,
                    QgsPostgresConn::quotedIdentifier( schema ),
                    QgsPostgresConn::quotedIdentifier( name ),
                    QgsPostgresConn::quotedIdentifier( options.geometryColumnName ) ) );
}

bool QgsPostgresProviderConnection::spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::SpatialIndexExists );

  const QList<QVariantList> res = executeSql( QStringLiteral( R"""(SELECT COUNT(*)
                                                              FROM pg_class t, pg_class i, pg_namespace ns, pg_index ix, pg_attribute a
                                                              WHERE
                                                                  t.oid=ix.indrelid
                                                                  AND t.relnamespace=ns.oid
                                                                  AND i.oid=ix.indexrelid
                                                                  AND a.attrelid=t.oid
                                                                  AND a.attnum=ANY(ix.indkey)
                                                                  AND t.relkind='r'
                                                                  AND ns.nspname=%1
                                                                  AND t.relname=%2
                                                                  AND a.attname=%3;
                                                              )""" ).arg(
                                    QgsPostgresConn::quotedValue( schema ),
                                    QgsPostgresConn::quotedValue( name ),
                                    QgsPostgresConn::quotedValue( geometryColumn ) ) );
  return !res.isEmpty() && !res.at( 0 ).isEmpty() && res.at( 0 ).at( 0 ).toBool();
}

void QgsPostgresProviderConnection::deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const
{
  checkCapability( Capability::DeleteSpatialIndex );

  const QList<QVariantList> res = executeSql( QStringLiteral( R"""(SELECT i.relname
                                                                FROM pg_class t, pg_class i, pg_namespace ns, pg_index ix, pg_attribute a
                                                                WHERE
                                                                    t.oid=ix.indrelid
                                                                    AND t.relnamespace=ns.oid
                                                                    AND i.oid=ix.indexrelid
                                                                    AND a.attrelid=t.oid
                                                                    AND a.attnum=ANY(ix.indkey)
                                                                    AND t.relkind='r'
                                                                    AND ns.nspname=%1
                                                                    AND t.relname=%2
                                                                    AND a.attname=%3;
                                                                )""" ).arg(
                                    QgsPostgresConn::quotedValue( schema ),
                                    QgsPostgresConn::quotedValue( name ),
                                    QgsPostgresConn::quotedValue( geometryColumn ) ) );
  if ( res.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "No spatial index exists for %1.%2" ).arg( schema, name ) );

  const QString indexName = res.at( 0 ).at( 0 ).toString();

  executeSql( QStringLiteral( "DROP INDEX %1.%2" ).arg( QgsPostgresConn::quotedIdentifier( schema ),
              QgsPostgresConn::quotedIdentifier( indexName ) ) );
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
      bool dontResolveType = configuration().value( QStringLiteral( "dontResolveType" ), false ).toBool();
      bool useEstimatedMetadata = configuration().value( QStringLiteral( "estimatedMetadata" ), false ).toBool();

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
        if ( pr.isForeignTable )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::Foreign );
        }
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
            conn->retrieveLayerTypes( pr, useEstimatedMetadata );
          }
          QgsPostgresProviderConnection::TableProperty property;
          property.setFlags( prFlags );
          for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ) ; i++ )
          {
            property.addGeometryColumnType( pr.types.at( i ), QgsCoordinateReferenceSystem::fromEpsgId( pr.srids.at( i ) ) );
          }
          property.setTableName( pr.tableName );
          property.setSchema( pr.schemaName );
          property.setGeometryColumn( pr.geometryColName );
          property.setPrimaryKeyColumns( pr.pkCols );
          property.setGeometryColumnCount( static_cast<int>( pr.nSpCols ) );
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


void QgsPostgresProviderConnection::store( const QString &name ) const
{
  // TODO: move this to class configuration?
  QString baseKey = QStringLiteral( "/PostgreSQL/connections/" );
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name );

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

  // From configuration
  static const QStringList configurationParameters
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
  for ( const auto &p : configurationParameters )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) );
    }
  }
  settings.endGroup();
  settings.endGroup();
}

void QgsPostgresProviderConnection::remove( const QString &name ) const
{
  QgsPostgresConn::deleteConnection( name );
}

QIcon QgsPostgresProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPostgis.svg" ) );
}

