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
#include "qgspostgresprovidermetadatautils.h"
#include "qgssettings.h"
#include "qgspostgresprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"
#include "qgsvectorlayer.h"
#include <QRegularExpression>
#include <QIcon>

#include <chrono>

extern "C"
{
#include <libpq-fe.h>
}

// From configuration
const QStringList QgsPostgresProviderConnection::CONFIGURATION_PARAMETERS =
{
  QStringLiteral( "publicOnly" ),
  QStringLiteral( "geometryColumnsOnly" ),
  QStringLiteral( "dontResolveType" ),
  QStringLiteral( "allowGeometrylessTables" ),
  QStringLiteral( "saveUsername" ),
  QStringLiteral( "savePassword" ),
  QStringLiteral( "estimatedMetadata" ),
  QStringLiteral( "projectsInDatabase" ),
  QStringLiteral( "metadataInDatabase" ),
};

const QString QgsPostgresProviderConnection::SETTINGS_BASE_KEY = QStringLiteral( "/PostgreSQL/connections/" );


QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "postgres" );
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
  setUri( QgsPostgresConn::connUri( name ).uri( false ).replace( removePartsRe, QString() ) );

  QgsSettings settings;
  settings.beginGroup( SETTINGS_BASE_KEY );
  settings.beginGroup( name );

  QVariantMap config;

  for ( const QString &p : std::as_const( CONFIGURATION_PARAMETERS ) )
  {
    const QVariant val = settings.value( p );
    if ( val.isValid() )
    {
      config.insert( p, val );
    }
  }

  settings.endGroup();
  settings.endGroup();

  setConfiguration( config );
  setDefaultCapabilities();
}

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QgsDataSourceUri( uri ).connectionInfo( false ), configuration )
{
  mProviderKey = QStringLiteral( "postgres" );
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
    Capability::DeleteSpatialIndex,
    Capability::DeleteField,
    Capability::DeleteFieldCascade,
    Capability::AddField
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePoint,
    GeometryColumnCapability::SingleLineString,
    GeometryColumnCapability::SinglePolygon,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities =
  {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds,
  };

  // see https://www.postgresql.org/docs/current/ddl-system-columns.html
  mIllegalFieldNames =
  {
    QStringLiteral( "tableoid" ),
    QStringLiteral( "xmin" ),
    QStringLiteral( "cmin" ),
    QStringLiteral( "xmax" ),
    QStringLiteral( "cmax" ),
    QStringLiteral( "ctid" ),

  };
}

void QgsPostgresProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  executeSqlPrivate( QStringLiteral( "DROP TABLE %1.%2" )
                     .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( name ) ) );
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
  Qgis::VectorExportResult res = QgsPostgresProvider::createEmptyLayer(
                                   newUri.uri(),
                                   fields,
                                   wkbType,
                                   srs,
                                   overwrite,
                                   &map,
                                   &errCause,
                                   options
                                 );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsPostgresProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
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
                     .arg( QgsPostgresConn::quotedIdentifier( schema ),
                           QgsPostgresConn::quotedIdentifier( name ),
                           QgsPostgresConn::quotedIdentifier( newName ) ) );
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
                     .arg( QgsPostgresConn::quotedIdentifier( name ),
                           force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

void QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlPrivate( QStringLiteral( "ALTER SCHEMA %1 RENAME TO %2" )
                     .arg( QgsPostgresConn::quotedIdentifier( name ),
                           QgsPostgresConn::quotedIdentifier( newName ) ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsPostgresProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return execSqlPrivate( sql, true, feedback );
}

QList<QVariantList> QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolPostgresConn> pgconn ) const
{
  return execSqlPrivate( sql, resolveTypes, feedback, pgconn ).rows();
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsPostgresProviderConnection::execSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolPostgresConn> pgconn ) const
{
  if ( ! pgconn )
  {
    pgconn = std::make_shared<QgsPoolPostgresConn>( QgsDataSourceUri( uri() ).connectionInfo( false ) );
  }

  std::shared_ptr<QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator> iterator = std::make_shared<QgsPostgresProviderResultIterator>( resolveTypes );
  QueryResult results( iterator );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QgsPostgresConn *conn = pgconn->get();

  if ( ! conn )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection failed: %1" ).arg( uri() ) );
  }
  else
  {

    if ( feedback && feedback->isCanceled() )
    {
      return results;
    }

    // This is gross but I tried with both conn and a context QObject without success: the lambda is never called.
    QMetaObject::Connection qtConnection;
    if ( feedback )
    {
      qtConnection = QObject::connect( feedback, &QgsFeedback::canceled, [ &pgconn ]
      {
        if ( pgconn )
          pgconn->get()->PQCancel();
      } );
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::unique_ptr<QgsPostgresResult> res = std::make_unique<QgsPostgresResult>( conn->LoggedPQexec( "QgsPostgresProviderConnection", sql ) );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );

    if ( feedback )
    {
      QObject::disconnect( qtConnection );
    }

    QString errCause;
    if ( conn->PQstatus() != CONNECTION_OK || ! res->result() )
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

    if ( ! errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }

    const qlonglong numRows { res->PQntuples() };

    if ( numRows > 0 )
    {

      // Get column names
      for ( int rowIdx = 0; rowIdx < res->PQnfields(); rowIdx++ )
      {
        results.appendColumn( res->PQfname( rowIdx ) );
      }

      // Try to convert value types at least for basic simple types that can be directly mapped to Python
      const int numFields { res->PQnfields() };
      if ( resolveTypes )
      {
        // Collect oids
        QStringList oids;
        oids.reserve( numFields );
        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          if ( feedback && feedback->isCanceled() )
          {
            break;
          }
          const QString oidStr { QString::number( res->PQftype( rowIdx ) ) };
          oids.push_back( oidStr );
        }

        const QList<QVariantList> typesResolved( executeSqlPrivate( QStringLiteral( "SELECT oid, typname FROM pg_type WHERE oid IN (%1)" ).arg( oids.join( ',' ) ), false, nullptr, pgconn ) );
        QgsStringMap oidTypeMap;
        for ( const auto &typeRes : std::as_const( typesResolved ) )
        {
          const QString oid { typeRes.constLast().toString() };
          if ( ! oidTypeMap.contains( oid ) )
          {
            oidTypeMap.insert( typeRes.constFirst().toString(), typeRes.constLast().toString() );
          }
        }

        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          static const QStringList intTypes = { QStringLiteral( "oid" ),
                                                QStringLiteral( "int2" ),
                                                QStringLiteral( "int4" ),
                                                QStringLiteral( "int8" ),
                                              };
          static const QStringList floatTypes = { QStringLiteral( "float4" ),
                                                  QStringLiteral( "float8" ),
                                                  QStringLiteral( "numeric" )
                                                };

          const QString typName { oidTypeMap[ oids.at( rowIdx )] };
          QVariant::Type vType { QVariant::Type::String };
          if ( floatTypes.contains( typName ) )
          {
            vType = QVariant::Double;
          }
          else if ( intTypes.contains( typName ) )
          {
            vType = QVariant::LongLong;
          }
          else if ( typName == QLatin1String( "date" ) )
          {
            vType = QVariant::Date;
          }
          else if ( typName.startsWith( QLatin1String( "timestamp" ) ) )
          {
            vType = QVariant::DateTime;
          }
          else if ( typName == QLatin1String( "time" ) )
          {
            vType = QVariant::Time;
          }
          else if ( typName == QLatin1String( "bool" ) )
          {
            vType = QVariant::Bool;
          }
          else if ( typName == QLatin1String( "char" ) )
          {
            vType = QVariant::Char;
          }
          else
          {
            // Just a warning, usually ok
            QgsDebugMsgLevel( QStringLiteral( "Unhandled PostgreSQL type %1, assuming string" ).arg( typName ), 2 );
          }
          static_cast<QgsPostgresProviderResultIterator *>( iterator.get() )->typeMap[ rowIdx ] = vType;
        }
      }
    }
    if ( ! errCause.isEmpty() )
    {
      throw QgsProviderConnectionException( errCause );
    }
    static_cast<QgsPostgresProviderResultIterator *>( iterator.get() )->result = std::move( res );
  }
  return results;
}


QVariantList QgsPostgresProviderResultIterator::nextRowPrivate()
{
  // Get results
  QVariantList row;

  if ( !result || mRowIndex >= result->PQntuples() )
  {
    return row;
  }

  row.reserve( result->PQnfields() );

  for ( int colIdx = 0; colIdx < result->PQnfields(); colIdx++ )
  {
    if ( mResolveTypes )
    {
      const QVariant::Type vType { typeMap.value( colIdx, QVariant::Type::String ) };
      QVariant val = result->PQgetvalue( mRowIndex, colIdx );
      // Special case for bools: 'f' and 't'
      if ( vType == QVariant::Bool )
      {
        const QString boolStrVal { val.toString() };
        if ( ! boolStrVal.isEmpty() )
        {
          val = boolStrVal == 't';
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
      row.push_back( result->PQgetvalue( mRowIndex, colIdx ) );
    }
  }
  ++mRowIndex;
  return row;
}

bool QgsPostgresProviderResultIterator::hasNextRowPrivate() const
{
  return result && mRowIndex < result->PQntuples();
}

long long QgsPostgresProviderResultIterator::rowCountPrivate() const
{
  return result ? result->PQntuples() : static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
}


void QgsPostgresProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::Vacuum );
  executeSql( QStringLiteral( "VACUUM FULL ANALYZE %1.%2" )
              .arg( QgsPostgresConn::quotedIdentifier( schema ),
                    QgsPostgresConn::quotedIdentifier( name ) ) );
}

void QgsPostgresProviderConnection::createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options ) const
{
  checkCapability( Capability::CreateSpatialIndex );

  QString geometryColumnName { options.geometryColumnName };
  if ( geometryColumnName.isEmpty() )
  {
    // Can we guess it?
    try
    {
      const auto tp { table( schema, name ) };
      geometryColumnName = tp.geometryColumn();
    }
    catch ( QgsProviderConnectionException & )
    {
      // pass
    }
  }

  if ( geometryColumnName.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Geometry column name not specified while creating spatial index" ) );
  }

  const QString indexName = QStringLiteral( "sidx_%1_%2" ).arg( name, geometryColumnName );
  executeSql( QStringLiteral( "CREATE INDEX %1 ON %2.%3 USING GIST (%4);" )
              .arg( QgsPostgresConn::quotedIdentifier( indexName ),
                    QgsPostgresConn::quotedIdentifier( schema ),
                    QgsPostgresConn::quotedIdentifier( name ),
                    QgsPostgresConn::quotedIdentifier( geometryColumnName ) ) );
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
                                                                  AND t.relkind IN ('r', 'm')
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
    QVector<QgsPostgresLayerProperty> properties;
    const bool aspatial { ! flags || flags.testFlag( TableFlag::Aspatial ) };
    bool ok = conn->supportedLayers( properties, false, schema == QStringLiteral( "public" ), aspatial, schema );
    if ( ! ok )
    {
      errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri() );
    }
    else
    {

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
          // These are candidates, not actual PKs
          // property.setPrimaryKeyColumns( pr.pkCols );
          property.setGeometryColumnCount( static_cast<int>( pr.nSpCols ) );
          property.setComment( pr.tableComment );

          // Get PKs
          if ( pr.isView || pr.isMaterializedView || pr.isForeignTable )
          {
            // Set the candidates
            property.setPrimaryKeyColumns( pr.pkCols );
          }
          else  // Fetch and set the real pks
          {
            try
            {
              const auto pks = executeSql( QStringLiteral( R"(
              WITH pkrelid AS (
              SELECT indexrelid AS idxri FROM pg_index WHERE indrelid='%1.%2'::regclass AND (indisprimary OR indisunique)
                ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1)
              SELECT attname FROM pg_index,pg_attribute, pkrelid
              WHERE indexrelid=pkrelid.idxri AND indrelid=attrelid AND pg_attribute.attnum=any(pg_index.indkey);
             )" ).arg( QgsPostgresConn::quotedIdentifier( pr.schemaName ),
                                               QgsPostgresConn::quotedIdentifier( pr.tableName ) ) );
              QStringList pkNames;
              for ( const auto &pk : std::as_const( pks ) )
              {
                pkNames.push_back( pk.first().toString() );
              }
              property.setPrimaryKeyColumns( pkNames );
            }
            catch ( const QgsProviderConnectionException &ex )
            {
              QgsDebugMsg( QStringLiteral( "Error retrieving primary keys: %1" ).arg( ex.what() ) );
            }
          }

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
      for ( const auto &s : std::as_const( schemaProperties ) )
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
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( SETTINGS_BASE_KEY );
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
  settings.setEnumValue( "sslmode", dsUri.sslMode() );

  for ( const auto &p : std::as_const( CONFIGURATION_PARAMETERS ) )
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


QList<QgsVectorDataProvider::NativeType> QgsPostgresProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsDataSourceUri{ uri() } .connectionInfo( false ) );
  if ( conn )
  {
    types = conn->nativeTypes();
    QgsPostgresConnPool::instance()->releaseConnection( conn );
  }
  if ( types.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for connection %1" ).arg( uri() ) );
  }
  return types;
}


QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsPostgresProviderConnection::sqlOptions( const QString &layerSource )
{
  SqlVectorLayerOptions options;
  const QgsDataSourceUri tUri( layerSource );
  options.primaryKeyColumns = tUri.keyColumn().split( ',' );
  options.disableSelectAtId = tUri.selectAtIdDisabled();
  options.geometryColumn = tUri.geometryColumn();
  options.filter = tUri.sql();
  const QString trimmedTable { tUri.table().trimmed() };
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : QStringLiteral( "SELECT * FROM %1" ).arg( tUri.quotedTablename() );
  return options;
}

QList<QgsLayerMetadataProviderResult> QgsPostgresProviderConnection::searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  return QgsPostgresProviderMetadataUtils::searchLayerMetadata( searchContext, uri(), searchString, geographicExtent, feedback );
}

QgsVectorLayer *QgsPostgresProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions &options ) const
{
  // Precondition
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri( ) );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( ! options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );
    tUri.setTable( QStringLiteral( "(%1)" ).arg( options.sql ) );
  }
  else
  {
    int pkId { 0 };
    while ( options.sql.contains( QStringLiteral( "_uid%1_" ).arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId ++;
    }
    tUri.setKeyColumn( QStringLiteral( "_uid%1_" ).arg( pkId ) );

    int sqlId { 0 };
    while ( options.sql.contains( QStringLiteral( "_subq_%1_" ).arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId ++;
    }
    tUri.setTable( QStringLiteral( "(SELECT row_number() over () AS _uid%1_, * FROM (%2\n) AS _subq_%3_\n)" ).arg( QString::number( pkId ), options.sql, QString::number( sqlId ) ) );
  }

  if ( ! options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer{ tUri.uri(), options.layerName.isEmpty() ? QStringLiteral( "QueryLayer" ) : options.layerName, providerKey(), vectorLayerOptions };
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsPostgresProviderConnection::sqlDictionary()
{
  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
  {
    {
      Qgis::SqlKeywordCategory::Keyword,
      {
        QStringLiteral( "absolute" ),
        QStringLiteral( "action" ),
        QStringLiteral( "add" ),
        QStringLiteral( "admin" ),
        QStringLiteral( "after" ),
        QStringLiteral( "aggregate" ),
        QStringLiteral( "alias" ),
        QStringLiteral( "all" ),
        QStringLiteral( "allocate" ),
        QStringLiteral( "alter" ),
        QStringLiteral( "analyse" ),
        QStringLiteral( "analyze" ),
        QStringLiteral( "and" ),
        QStringLiteral( "any" ),
        QStringLiteral( "are" ),
        QStringLiteral( "array" ),
        QStringLiteral( "as" ),
        QStringLiteral( "asc" ),
        QStringLiteral( "asensitive" ),
        QStringLiteral( "assertion" ),
        QStringLiteral( "asymmetric" ),
        QStringLiteral( "at" ),
        QStringLiteral( "atomic" ),
        QStringLiteral( "authorization" ),
        QStringLiteral( "avg" ),
        QStringLiteral( "before" ),
        QStringLiteral( "begin" ),
        QStringLiteral( "between" ),
        QStringLiteral( "bigint" ),
        QStringLiteral( "binary" ),
        QStringLiteral( "bit" ),
        QStringLiteral( "bit_length" ),
        QStringLiteral( "blob" ),
        QStringLiteral( "boolean" ),
        QStringLiteral( "both" ),
        QStringLiteral( "breadth" ),
        QStringLiteral( "by" ),
        QStringLiteral( "call" ),
        QStringLiteral( "called" ),
        QStringLiteral( "cardinality" ),
        QStringLiteral( "cascade" ),
        QStringLiteral( "cascaded" ),
        QStringLiteral( "case" ),
        QStringLiteral( "cast" ),
        QStringLiteral( "catalog" ),
        QStringLiteral( "ceil" ),
        QStringLiteral( "ceiling" ),
        QStringLiteral( "char" ),
        QStringLiteral( "character" ),
        QStringLiteral( "character_length" ),
        QStringLiteral( "char_length" ),
        QStringLiteral( "check" ),
        QStringLiteral( "class" ),
        QStringLiteral( "clob" ),
        QStringLiteral( "close" ),
        QStringLiteral( "coalesce" ),
        QStringLiteral( "collate" ),
        QStringLiteral( "collation" ),
        QStringLiteral( "collect" ),
        QStringLiteral( "column" ),
        QStringLiteral( "commit" ),
        QStringLiteral( "completion" ),
        QStringLiteral( "condition" ),
        QStringLiteral( "connect" ),
        QStringLiteral( "connection" ),
        QStringLiteral( "constraint" ),
        QStringLiteral( "constraints" ),
        QStringLiteral( "constructor" ),
        QStringLiteral( "continue" ),
        QStringLiteral( "convert" ),
        QStringLiteral( "corr" ),
        QStringLiteral( "corresponding" ),
        QStringLiteral( "count" ),
        QStringLiteral( "covar_pop" ),
        QStringLiteral( "covar_samp" ),
        QStringLiteral( "create" ),
        QStringLiteral( "cross" ),
        QStringLiteral( "cube" ),
        QStringLiteral( "cume_dist" ),
        QStringLiteral( "current" ),
        QStringLiteral( "current_date" ),
        QStringLiteral( "current_default_transform_group" ),
        QStringLiteral( "current_path" ),
        QStringLiteral( "current_role" ),
        QStringLiteral( "current_time" ),
        QStringLiteral( "current_timestamp" ),
        QStringLiteral( "current_transform_group_for_type" ),
        QStringLiteral( "current_user" ),
        QStringLiteral( "cursor" ),
        QStringLiteral( "cycle" ),
        QStringLiteral( "data" ),
        QStringLiteral( "date" ),
        QStringLiteral( "day" ),
        QStringLiteral( "deallocate" ),
        QStringLiteral( "dec" ),
        QStringLiteral( "decimal" ),
        QStringLiteral( "declare" ),
        QStringLiteral( "default" ),
        QStringLiteral( "deferrable" ),
        QStringLiteral( "deferred" ),
        QStringLiteral( "delete" ),
        QStringLiteral( "dense_rank" ),
        QStringLiteral( "depth" ),
        QStringLiteral( "deref" ),
        QStringLiteral( "desc" ),
        QStringLiteral( "describe" ),
        QStringLiteral( "descriptor" ),
        QStringLiteral( "destroy" ),
        QStringLiteral( "destructor" ),
        QStringLiteral( "deterministic" ),
        QStringLiteral( "diagnostics" ),
        QStringLiteral( "dictionary" ),
        QStringLiteral( "disconnect" ),
        QStringLiteral( "distinct" ),
        QStringLiteral( "do" ),
        QStringLiteral( "domain" ),
        QStringLiteral( "double" ),
        QStringLiteral( "drop" ),
        QStringLiteral( "dynamic" ),
        QStringLiteral( "each" ),
        QStringLiteral( "element" ),
        QStringLiteral( "else" ),
        QStringLiteral( "end" ),
        QStringLiteral( "end-exec" ),
        QStringLiteral( "equals" ),
        QStringLiteral( "escape" ),
        QStringLiteral( "every" ),
        QStringLiteral( "except" ),
        QStringLiteral( "exception" ),
        QStringLiteral( "exec" ),
        QStringLiteral( "execute" ),
        QStringLiteral( "exists" ),
        QStringLiteral( "exp" ),
        QStringLiteral( "external" ),
        QStringLiteral( "extract" ),
        QStringLiteral( "false" ),
        QStringLiteral( "fetch" ),
        QStringLiteral( "filter" ),
        QStringLiteral( "first" ),
        QStringLiteral( "float" ),
        QStringLiteral( "floor" ),
        QStringLiteral( "for" ),
        QStringLiteral( "foreign" ),
        QStringLiteral( "found" ),
        QStringLiteral( "free" ),
        QStringLiteral( "freeze" ),
        QStringLiteral( "from" ),
        QStringLiteral( "full" ),
        QStringLiteral( "function" ),
        QStringLiteral( "fusion" ),
        QStringLiteral( "general" ),
        QStringLiteral( "get" ),
        QStringLiteral( "global" ),
        QStringLiteral( "go" ),
        QStringLiteral( "goto" ),
        QStringLiteral( "grant" ),
        QStringLiteral( "group" ),
        QStringLiteral( "grouping" ),
        QStringLiteral( "having" ),
        QStringLiteral( "hold" ),
        QStringLiteral( "host" ),
        QStringLiteral( "hour" ),
        QStringLiteral( "identity" ),
        QStringLiteral( "ignore" ),
        QStringLiteral( "ilike" ),
        QStringLiteral( "immediate" ),
        QStringLiteral( "in" ),
        QStringLiteral( "indicator" ),
        QStringLiteral( "initialize" ),
        QStringLiteral( "initially" ),
        QStringLiteral( "inner" ),
        QStringLiteral( "inout" ),
        QStringLiteral( "input" ),
        QStringLiteral( "insensitive" ),
        QStringLiteral( "insert" ),
        QStringLiteral( "int" ),
        QStringLiteral( "integer" ),
        QStringLiteral( "intersect" ),
        QStringLiteral( "intersection" ),
        QStringLiteral( "interval" ),
        QStringLiteral( "into" ),
        QStringLiteral( "is" ),
        QStringLiteral( "isnull" ),
        QStringLiteral( "isolation" ),
        QStringLiteral( "iterate" ),
        QStringLiteral( "join" ),
        QStringLiteral( "key" ),
        QStringLiteral( "language" ),
        QStringLiteral( "large" ),
        QStringLiteral( "last" ),
        QStringLiteral( "lateral" ),
        QStringLiteral( "leading" ),
        QStringLiteral( "left" ),
        QStringLiteral( "less" ),
        QStringLiteral( "level" ),
        QStringLiteral( "like" ),
        QStringLiteral( "limit" ),
        QStringLiteral( "ln" ),
        QStringLiteral( "local" ),
        QStringLiteral( "localtime" ),
        QStringLiteral( "localtimestamp" ),
        QStringLiteral( "locator" ),
        QStringLiteral( "lower" ),
        QStringLiteral( "map" ),
        QStringLiteral( "match" ),
        QStringLiteral( "max" ),
        QStringLiteral( "member" ),
        QStringLiteral( "merge" ),
        QStringLiteral( "method" ),
        QStringLiteral( "min" ),
        QStringLiteral( "minute" ),
        QStringLiteral( "mod" ),
        QStringLiteral( "modifies" ),
        QStringLiteral( "modify" ),
        QStringLiteral( "module" ),
        QStringLiteral( "month" ),
        QStringLiteral( "multiset" ),
        QStringLiteral( "names" ),
        QStringLiteral( "national" ),
        QStringLiteral( "natural" ),
        QStringLiteral( "nchar" ),
        QStringLiteral( "nclob" ),
        QStringLiteral( "new" ),
        QStringLiteral( "next" ),
        QStringLiteral( "no" ),
        QStringLiteral( "none" ),
        QStringLiteral( "normalize" ),
        QStringLiteral( "not" ),
        QStringLiteral( "notnull" ),
        QStringLiteral( "null" ),
        QStringLiteral( "nullif" ),
        QStringLiteral( "numeric" ),
        QStringLiteral( "object" ),
        QStringLiteral( "octet_length" ),
        QStringLiteral( "of" ),
        QStringLiteral( "off" ),
        QStringLiteral( "offset" ),
        QStringLiteral( "old" ),
        QStringLiteral( "on" ),
        QStringLiteral( "only" ),
        QStringLiteral( "open" ),
        QStringLiteral( "operation" ),
        QStringLiteral( "option" ),
        QStringLiteral( "or" ),
        QStringLiteral( "order" ),
        QStringLiteral( "ordinality" ),
        QStringLiteral( "out" ),
        QStringLiteral( "outer" ),
        QStringLiteral( "output" ),
        QStringLiteral( "over" ),
        QStringLiteral( "overlaps" ),
        QStringLiteral( "overlay" ),
        QStringLiteral( "pad" ),
        QStringLiteral( "parameter" ),
        QStringLiteral( "parameters" ),
        QStringLiteral( "partial" ),
        QStringLiteral( "partition" ),
        QStringLiteral( "path" ),
        QStringLiteral( "percentile_cont" ),
        QStringLiteral( "percentile_disc" ),
        QStringLiteral( "percent_rank" ),
        QStringLiteral( "placing" ),
        QStringLiteral( "position" ),
        QStringLiteral( "postfix" ),
        QStringLiteral( "power" ),
        QStringLiteral( "precision" ),
        QStringLiteral( "prefix" ),
        QStringLiteral( "preorder" ),
        QStringLiteral( "prepare" ),
        QStringLiteral( "preserve" ),
        QStringLiteral( "primary" ),
        QStringLiteral( "prior" ),
        QStringLiteral( "privileges" ),
        QStringLiteral( "procedure" ),
        QStringLiteral( "public" ),
        QStringLiteral( "range" ),
        QStringLiteral( "rank" ),
        QStringLiteral( "read" ),
        QStringLiteral( "reads" ),
        QStringLiteral( "real" ),
        QStringLiteral( "recursive" ),
        QStringLiteral( "ref" ),
        QStringLiteral( "references" ),
        QStringLiteral( "referencing" ),
        QStringLiteral( "regr_avgx" ),
        QStringLiteral( "regr_avgy" ),
        QStringLiteral( "regr_count" ),
        QStringLiteral( "regr_intercept" ),
        QStringLiteral( "regr_r2" ),
        QStringLiteral( "regr_slope" ),
        QStringLiteral( "regr_sxx" ),
        QStringLiteral( "regr_sxy" ),
        QStringLiteral( "regr_syy" ),
        QStringLiteral( "relative" ),
        QStringLiteral( "release" ),
        QStringLiteral( "restrict" ),
        QStringLiteral( "result" ),
        QStringLiteral( "return" ),
        QStringLiteral( "returning" ),
        QStringLiteral( "returns" ),
        QStringLiteral( "revoke" ),
        QStringLiteral( "right" ),
        QStringLiteral( "role" ),
        QStringLiteral( "rollback" ),
        QStringLiteral( "rollup" ),
        QStringLiteral( "routine" ),
        QStringLiteral( "row" ),
        QStringLiteral( "row_number" ),
        QStringLiteral( "rows" ),
        QStringLiteral( "savepoint" ),
        QStringLiteral( "schema" ),
        QStringLiteral( "scope" ),
        QStringLiteral( "scroll" ),
        QStringLiteral( "search" ),
        QStringLiteral( "second" ),
        QStringLiteral( "section" ),
        QStringLiteral( "select" ),
        QStringLiteral( "sensitive" ),
        QStringLiteral( "sequence" ),
        QStringLiteral( "session" ),
        QStringLiteral( "session_user" ),
        QStringLiteral( "set" ),
        QStringLiteral( "sets" ),
        QStringLiteral( "similar" ),
        QStringLiteral( "size" ),
        QStringLiteral( "smallint" ),
        QStringLiteral( "some" ),
        QStringLiteral( "space" ),
        QStringLiteral( "specific" ),
        QStringLiteral( "specifictype" ),
        QStringLiteral( "sql" ),
        QStringLiteral( "sqlcode" ),
        QStringLiteral( "sqlerror" ),
        QStringLiteral( "sqlexception" ),
        QStringLiteral( "sqlstate" ),
        QStringLiteral( "sqlwarning" ),
        QStringLiteral( "sqrt" ),
        QStringLiteral( "start" ),
        QStringLiteral( "state" ),
        QStringLiteral( "statement" ),
        QStringLiteral( "static" ),
        QStringLiteral( "stddev_pop" ),
        QStringLiteral( "stddev_samp" ),
        QStringLiteral( "structure" ),
        QStringLiteral( "submultiset" ),
        QStringLiteral( "substring" ),
        QStringLiteral( "sum" ),
        QStringLiteral( "symmetric" ),
        QStringLiteral( "system" ),
        QStringLiteral( "system_user" ),
        QStringLiteral( "table" ),
        QStringLiteral( "tablesample" ),
        QStringLiteral( "temporary" ),
        QStringLiteral( "terminate" ),
        QStringLiteral( "than" ),
        QStringLiteral( "then" ),
        QStringLiteral( "time" ),
        QStringLiteral( "timestamp" ),
        QStringLiteral( "timezone_hour" ),
        QStringLiteral( "timezone_minute" ),
        QStringLiteral( "to" ),
        QStringLiteral( "trailing" ),
        QStringLiteral( "transaction" ),
        QStringLiteral( "translate" ),
        QStringLiteral( "translation" ),
        QStringLiteral( "treat" ),
        QStringLiteral( "trigger" ),
        QStringLiteral( "trim" ),
        QStringLiteral( "true" ),
        QStringLiteral( "uescape" ),
        QStringLiteral( "under" ),
        QStringLiteral( "union" ),
        QStringLiteral( "unique" ),
        QStringLiteral( "unknown" ),
        QStringLiteral( "unnest" ),
        QStringLiteral( "update" ),
        QStringLiteral( "upper" ),
        QStringLiteral( "usage" ),
        QStringLiteral( "user" ),
        QStringLiteral( "using" ),
        QStringLiteral( "value" ),
        QStringLiteral( "values" ),
        QStringLiteral( "varchar" ),
        QStringLiteral( "variable" ),
        QStringLiteral( "var_pop" ),
        QStringLiteral( "var_samp" ),
        QStringLiteral( "varying" ),
        QStringLiteral( "verbose" ),
        QStringLiteral( "view" ),
        QStringLiteral( "when" ),
        QStringLiteral( "whenever" ),
        QStringLiteral( "where" ),
        QStringLiteral( "width_bucket" ),
        QStringLiteral( "window" ),
        QStringLiteral( "with" ),
        QStringLiteral( "within" ),
        QStringLiteral( "without" ),
        QStringLiteral( "work" ),
        QStringLiteral( "write" ),
        QStringLiteral( "xml" ),
        QStringLiteral( "xmlagg" ),
        QStringLiteral( "xmlattributes" ),
        QStringLiteral( "xmlbinary" ),
        QStringLiteral( "xmlcomment" ),
        QStringLiteral( "xmlconcat" ),
        QStringLiteral( "xmlelement" ),
        QStringLiteral( "xmlforest" ),
        QStringLiteral( "xmlnamespaces" ),
        QStringLiteral( "xmlparse" ),
        QStringLiteral( "xmlpi" ),
        QStringLiteral( "xmlroot" ),
        QStringLiteral( "xmlserialize" ),
        QStringLiteral( "year" ),
        QStringLiteral( "zone" ),
      }
    },
    {
      Qgis::SqlKeywordCategory::Aggregate,
      {
        QStringLiteral( "Max" ),
        QStringLiteral( "Min" ),
        QStringLiteral( "Avg" ),
        QStringLiteral( "Count" ),
        QStringLiteral( "Sum" ),
        QStringLiteral( "Group_Concat" ),
        QStringLiteral( "Total" ),
        QStringLiteral( "Var_Pop" ),
        QStringLiteral( "Var_Samp" ),
        QStringLiteral( "StdDev_Pop" ),
        QStringLiteral( "StdDev_Samp" ),
      }
    },
    {
      Qgis::SqlKeywordCategory::Math,
      {
        QStringLiteral( "Abs" ),
        QStringLiteral( "ACos" ),
        QStringLiteral( "ASin" ),
        QStringLiteral( "ATan" ),
        QStringLiteral( "Cos" ),
        QStringLiteral( "Cot" ),
        QStringLiteral( "Degrees" ),
        QStringLiteral( "Exp" ),
        QStringLiteral( "Floor" ),
        QStringLiteral( "Log" ),
        QStringLiteral( "Log2" ),

        QStringLiteral( "Log10" ),
        QStringLiteral( "Pi" ),
        QStringLiteral( "Radians" ),
        QStringLiteral( "Round" ),
        QStringLiteral( "Sign" ),
        QStringLiteral( "Sin" ),
        QStringLiteral( "Sqrt" ),
        QStringLiteral( "StdDev_Pop" ),
        QStringLiteral( "StdDev_Samp" ),
        QStringLiteral( "Tan" ),
        QStringLiteral( "Var_Pop" ),
        QStringLiteral( "Var_Samp" ),
      }
    },
    {
      Qgis::SqlKeywordCategory::Geospatial,
      {
        // List from:
        // import requests, re, html;
        // result = requests.get('https://postgis.net/docs/PostGIS_Special_Functions_Index.html')
        // m = re.findall('<a class="link".*?title="(.*?)">.*?</li>', result.content.replace(b'\xc2', b'').replace(b'\xa0', b'').decode('utf8'), re.MULTILINE)
        // for f_name in sorted(set(m)):
        //     print(f'              QStringLiteral( "{html.unescape(f_name)}" ),')

        QStringLiteral( "&&" ),
        QStringLiteral( "&&&" ),
        QStringLiteral( "&&&(geometry,gidx)" ),
        QStringLiteral( "&&&(gidx,geometry)" ),
        QStringLiteral( "&&&(gidx,gidx)" ),
        QStringLiteral( "&&(box2df,box2df)" ),
        QStringLiteral( "&&(box2df,geometry)" ),
        QStringLiteral( "&&(geometry,box2df)" ),
        QStringLiteral( "&>" ),
        QStringLiteral( "&<" ),
        QStringLiteral( "&<|" ),
        QStringLiteral( "<#>" ),
        QStringLiteral( "<<#>>" ),
        QStringLiteral( "<<->>" ),
        QStringLiteral( "<->" ),
        QStringLiteral( "=" ),
        QStringLiteral( "@" ),
        QStringLiteral( "@(box2df,box2df)" ),
        QStringLiteral( "@(box2df,geometry)" ),
        QStringLiteral( "@(geometry,box2df)" ),
        QStringLiteral( "AddEdge" ),
        QStringLiteral( "AddFace" ),
        QStringLiteral( "AddGeometryColumn" ),
        QStringLiteral( "AddNode" ),
        QStringLiteral( "AddOverviewConstraints" ),
        QStringLiteral( "AddRasterConstraints" ),
        QStringLiteral( "AsGML" ),
        QStringLiteral( "AsTopoJSON" ),
        QStringLiteral( "Box2D" ),
        QStringLiteral( "Box3D" ),
        QStringLiteral( "CopyTopology" ),
        QStringLiteral( "DropGeometryColumn" ),
        QStringLiteral( "DropOverviewConstraints" ),
        QStringLiteral( "DropRasterConstraints" ),
        QStringLiteral( "Drop_Indexes_Generate_Script" ),
        QStringLiteral( "Drop_Nation_Tables_Generate_Script" ),
        QStringLiteral( "Drop_State_Tables_Generate_Script" ),
        QStringLiteral( "Equals" ),
        QStringLiteral( "Geocode" ),
        QStringLiteral( "Geocode_Intersection" ),
        QStringLiteral( "GeometryType" ),
        QStringLiteral( "GetEdgeByPoint" ),
        QStringLiteral( "GetFaceByPoint" ),
        QStringLiteral( "GetNodeByPoint" ),
        QStringLiteral( "GetNodeEdges" ),
        QStringLiteral( "GetRingEdges" ),
        QStringLiteral( "GetTopoGeomElements" ),
        QStringLiteral( "GetTopologySRID" ),
        QStringLiteral( "Get_Geocode_Setting" ),
        QStringLiteral( "Get_Tract" ),
        QStringLiteral( "Install_Missing_Indexes" ),
        QStringLiteral( "Intersects" ),
        QStringLiteral( "Loader_Generate_Census_Script" ),
        QStringLiteral( "Loader_Generate_Nation_Script" ),
        QStringLiteral( "Loader_Generate_Script" ),
        QStringLiteral( "Missing_Indexes_Generate_Script" ),
        QStringLiteral( "Normalize_Address" ),
        QStringLiteral( "Pagc_Normalize_Address" ),
        QStringLiteral( "Polygonize" ),
        QStringLiteral( "Populate_Geometry_Columns" ),
        QStringLiteral( "Populate_Topology_Layer" ),
        QStringLiteral( "PostGIS_AddBBox" ),
        QStringLiteral( "PostGIS_DropBBox" ),
        QStringLiteral( "PostGIS_Extensions_Upgrade" ),
        QStringLiteral( "PostGIS_HasBBox" ),
        QStringLiteral( "PostGIS_LibXML_Version" ),
        QStringLiteral( "Reverse_Geocode" ),
        QStringLiteral( "ST_3DArea" ),
        QStringLiteral( "ST_3DClosestPoint" ),
        QStringLiteral( "ST_3DDFullyWithin" ),
        QStringLiteral( "ST_3DDWithin" ),
        QStringLiteral( "ST_3DDifference" ),
        QStringLiteral( "ST_3DDistance" ),
        QStringLiteral( "ST_3DExtent" ),
        QStringLiteral( "ST_3DIntersection" ),
        QStringLiteral( "ST_3DIntersects" ),
        QStringLiteral( "ST_3DLength" ),
        QStringLiteral( "ST_3DLineInterpolatePoint" ),
        QStringLiteral( "ST_3DLongestLine" ),
        QStringLiteral( "ST_3DMakeBox" ),
        QStringLiteral( "ST_3DMaxDistance" ),
        QStringLiteral( "ST_3DPerimeter" ),
        QStringLiteral( "ST_3DShortestLine" ),
        QStringLiteral( "ST_3DUnion" ),
        QStringLiteral( "ST_AddBand" ),
        QStringLiteral( "ST_AddEdgeModFace" ),
        QStringLiteral( "ST_AddEdgeNewFaces" ),
        QStringLiteral( "ST_AddMeasure" ),
        QStringLiteral( "ST_AddPoint" ),
        QStringLiteral( "ST_Affine" ),
        QStringLiteral( "ST_Angle" ),
        QStringLiteral( "ST_ApproximateMedialAxis" ),
        QStringLiteral( "ST_Area" ),
        QStringLiteral( "ST_AsBinary" ),
        QStringLiteral( "ST_AsBinary/ST_AsWKB" ),
        QStringLiteral( "ST_AsEWKB" ),
        QStringLiteral( "ST_AsEWKT" ),
        QStringLiteral( "ST_AsEncodedPolyline" ),
        QStringLiteral( "ST_AsGDALRaster" ),
        QStringLiteral( "ST_AsGML" ),
        QStringLiteral( "ST_AsGeoJSON" ),
        QStringLiteral( "ST_AsGeobuf" ),
        QStringLiteral( "ST_AsHEXEWKB" ),
        QStringLiteral( "ST_AsHexWKB" ),
        QStringLiteral( "ST_AsJPEG" ),
        QStringLiteral( "ST_AsKML" ),
        QStringLiteral( "ST_AsLatLonText" ),
        QStringLiteral( "ST_AsMVT" ),
        QStringLiteral( "ST_AsMVTGeom" ),
        QStringLiteral( "ST_AsPNG" ),
        QStringLiteral( "ST_AsRaster" ),
        QStringLiteral( "ST_AsSVG" ),
        QStringLiteral( "ST_AsTIFF" ),
        QStringLiteral( "ST_AsTWKB" ),
        QStringLiteral( "ST_AsText" ),
        QStringLiteral( "ST_AsX3D" ),
        QStringLiteral( "ST_Aspect" ),
        QStringLiteral( "ST_Azimuth" ),
        QStringLiteral( "ST_Band" ),
        QStringLiteral( "ST_BandFileSize" ),
        QStringLiteral( "ST_BandFileTimestamp" ),
        QStringLiteral( "ST_BandIsNoData" ),
        QStringLiteral( "ST_BandMetaData" ),
        QStringLiteral( "ST_BandNoDataValue" ),
        QStringLiteral( "ST_BandPath" ),
        QStringLiteral( "ST_BandPixelType" ),
        QStringLiteral( "ST_Boundary" ),
        QStringLiteral( "ST_BoundingDiagonal" ),
        QStringLiteral( "ST_Box2dFromGeoHash" ),
        QStringLiteral( "ST_Buffer" ),
        QStringLiteral( "ST_CPAWithin" ),
        QStringLiteral( "ST_Centroid" ),
        QStringLiteral( "ST_ChaikinSmoothing" ),
        QStringLiteral( "ST_Clip" ),
        QStringLiteral( "ST_ClipByBox2D" ),
        QStringLiteral( "ST_ClosestPoint" ),
        QStringLiteral( "ST_ClosestPointOfApproach" ),
        QStringLiteral( "ST_ClusterDBSCAN" ),
        QStringLiteral( "ST_ClusterIntersecting" ),
        QStringLiteral( "ST_ClusterKMeans" ),
        QStringLiteral( "ST_ClusterWithin" ),
        QStringLiteral( "ST_Collect" ),
        QStringLiteral( "ST_CollectionExtract" ),
        QStringLiteral( "ST_CollectionHomogenize" ),
        QStringLiteral( "ST_ColorMap" ),
        QStringLiteral( "ST_ConcaveHull" ),
        QStringLiteral( "ST_ConstrainedDelaunayTriangles" ),
        QStringLiteral( "ST_Contains" ),
        QStringLiteral( "ST_ContainsProperly" ),
        QStringLiteral( "ST_ConvexHull" ),
        QStringLiteral( "ST_CoordDim" ),
        QStringLiteral( "ST_Count" ),
        QStringLiteral( "ST_CountAgg" ),
        QStringLiteral( "ST_CoveredBy" ),
        QStringLiteral( "ST_Covers" ),
        QStringLiteral( "ST_CreateOverview" ),
        QStringLiteral( "ST_CreateTopoGeo" ),
        QStringLiteral( "ST_Crosses" ),
        QStringLiteral( "ST_CurveToLine" ),
        QStringLiteral( "ST_DFullyWithin" ),
        QStringLiteral( "ST_DWithin" ),
        QStringLiteral( "ST_DelaunayTriangles" ),
        QStringLiteral( "ST_Difference" ),
        QStringLiteral( "ST_Dimension" ),
        QStringLiteral( "ST_Disjoint" ),
        QStringLiteral( "ST_Distance" ),
        QStringLiteral( "ST_DistanceCPA" ),
        QStringLiteral( "ST_DistanceSphere" ),
        QStringLiteral( "ST_DistanceSpheroid" ),
        QStringLiteral( "ST_Distinct4ma" ),
        QStringLiteral( "ST_Dump" ),
        QStringLiteral( "ST_DumpAsPolygons" ),
        QStringLiteral( "ST_DumpPoints" ),
        QStringLiteral( "ST_DumpRings" ),
        QStringLiteral( "ST_DumpValues" ),
        QStringLiteral( "ST_EndPoint" ),
        QStringLiteral( "ST_Envelope" ),
        QStringLiteral( "ST_Equals" ),
        QStringLiteral( "ST_EstimatedExtent" ),
        QStringLiteral( "ST_Expand" ),
        QStringLiteral( "ST_Extent" ),
        QStringLiteral( "ST_ExteriorRing" ),
        QStringLiteral( "ST_Extrude" ),
        QStringLiteral( "ST_FilterByM" ),
        QStringLiteral( "ST_FlipCoordinates" ),
        QStringLiteral( "ST_Force2D" ),
        QStringLiteral( "ST_Force3D" ),
        QStringLiteral( "ST_Force3DM" ),
        QStringLiteral( "ST_Force3DZ" ),
        QStringLiteral( "ST_Force4D" ),
        QStringLiteral( "ST_ForceCollection" ),
        QStringLiteral( "ST_ForceCurve" ),
        QStringLiteral( "ST_ForceLHR" ),
        QStringLiteral( "ST_ForcePolygonCCW" ),
        QStringLiteral( "ST_ForcePolygonCW" ),
        QStringLiteral( "ST_ForceRHR" ),
        QStringLiteral( "ST_ForceSFS" ),
        QStringLiteral( "ST_FrechetDistance" ),
        QStringLiteral( "ST_FromGDALRaster" ),
        QStringLiteral( "ST_GDALDrivers" ),
        QStringLiteral( "ST_GMLToSQL" ),
        QStringLiteral( "ST_GeneratePoints" ),
        QStringLiteral( "ST_GeoHash" ),
        QStringLiteral( "ST_GeoReference" ),
        QStringLiteral( "ST_GeogFromText" ),
        QStringLiteral( "ST_GeogFromWKB" ),
        QStringLiteral( "ST_GeographyFromText" ),
        QStringLiteral( "ST_GeomFromEWKB" ),
        QStringLiteral( "ST_GeomFromEWKT" ),
        QStringLiteral( "ST_GeomFromGML" ),
        QStringLiteral( "ST_GeomFromGeoHash" ),
        QStringLiteral( "ST_GeomFromGeoJSON" ),
        QStringLiteral( "ST_GeomFromKML" ),
        QStringLiteral( "ST_GeomFromText" ),
        QStringLiteral( "ST_GeomFromWKB" ),
        QStringLiteral( "ST_GeometricMedian" ),
        QStringLiteral( "ST_GeometryN" ),
        QStringLiteral( "ST_GeometryType" ),
        QStringLiteral( "ST_GetFaceEdges" ),
        QStringLiteral( "ST_Grayscale" ),
        QStringLiteral( "ST_HasArc" ),
        QStringLiteral( "ST_HasNoBand" ),
        QStringLiteral( "ST_HausdorffDistance" ),
        QStringLiteral( "ST_Height" ),
        QStringLiteral( "ST_Hexagon" ),
        QStringLiteral( "ST_HexagonGrid" ),
        QStringLiteral( "ST_HillShade" ),
        QStringLiteral( "ST_Histogram" ),
        QStringLiteral( "ST_InteriorRingN" ),
        QStringLiteral( "ST_InterpolatePoint" ),
        QStringLiteral( "ST_Intersection" ),
        QStringLiteral( "ST_Intersects" ),
        QStringLiteral( "ST_InvDistWeight4ma" ),
        QStringLiteral( "ST_IsClosed" ),
        QStringLiteral( "ST_IsCollection" ),
        QStringLiteral( "ST_IsEmpty" ),
        QStringLiteral( "ST_IsPlanar" ),
        QStringLiteral( "ST_IsPolygonCCW" ),
        QStringLiteral( "ST_IsPolygonCW" ),
        QStringLiteral( "ST_IsSimple" ),
        QStringLiteral( "ST_IsSolid" ),
        QStringLiteral( "ST_IsValidDetail" ),
        QStringLiteral( "ST_IsValidReason" ),
        QStringLiteral( "ST_IsValidTrajectory" ),
        QStringLiteral( "ST_Length" ),
        QStringLiteral( "ST_LengthSpheroid" ),
        QStringLiteral( "ST_LineCrossingDirection" ),
        QStringLiteral( "ST_LineFromEncodedPolyline" ),
        QStringLiteral( "ST_LineFromMultiPoint" ),
        QStringLiteral( "ST_LineInterpolatePoint" ),
        QStringLiteral( "ST_LineInterpolatePoints" ),
        QStringLiteral( "ST_LineLocatePoint" ),
        QStringLiteral( "ST_LineSubstring" ),
        QStringLiteral( "ST_LineToCurve" ),
        QStringLiteral( "ST_LocateAlong" ),
        QStringLiteral( "ST_LocateBetween" ),
        QStringLiteral( "ST_LocateBetweenElevations" ),
        QStringLiteral( "ST_LongestLine" ),
        QStringLiteral( "ST_M" ),
        QStringLiteral( "ST_MakeBox2D" ),
        QStringLiteral( "ST_MakeEmptyCoverage" ),
        QStringLiteral( "ST_MakeEmptyRaster" ),
        QStringLiteral( "ST_MakeEnvelope" ),
        QStringLiteral( "ST_MakeLine" ),
        QStringLiteral( "ST_MakePoint" ),
        QStringLiteral( "ST_MakePolygon" ),
        QStringLiteral( "ST_MakeSolid" ),
        QStringLiteral( "ST_MakeValid" ),
        QStringLiteral( "ST_MapAlgebra (callback function version)" ),
        QStringLiteral( "ST_MapAlgebra (expression version)" ),
        QStringLiteral( "ST_MapAlgebraExpr" ),
        QStringLiteral( "ST_MapAlgebraFct" ),
        QStringLiteral( "ST_MapAlgebraFctNgb" ),
        QStringLiteral( "ST_Max4ma" ),
        QStringLiteral( "ST_MaxDistance" ),
        QStringLiteral( "ST_MaximumInscribedCircle" ),
        QStringLiteral( "ST_Mean4ma" ),
        QStringLiteral( "ST_MemSize" ),
        QStringLiteral( "ST_MemUnion" ),
        QStringLiteral( "ST_MetaData" ),
        QStringLiteral( "ST_Min4ma" ),
        QStringLiteral( "ST_MinConvexHull" ),
        QStringLiteral( "ST_MinDist4ma" ),
        QStringLiteral( "ST_MinimumBoundingCircle" ),
        QStringLiteral( "ST_MinimumClearance" ),
        QStringLiteral( "ST_MinimumClearanceLine" ),
        QStringLiteral( "ST_MinkowskiSum" ),
        QStringLiteral( "ST_ModEdgeHeal" ),
        QStringLiteral( "ST_ModEdgeSplit" ),
        QStringLiteral( "ST_NDims" ),
        QStringLiteral( "ST_NPoints" ),
        QStringLiteral( "ST_NRings" ),
        QStringLiteral( "ST_NearestValue" ),
        QStringLiteral( "ST_Neighborhood" ),
        QStringLiteral( "ST_NewEdgeHeal" ),
        QStringLiteral( "ST_Node" ),
        QStringLiteral( "ST_Normalize" ),
        QStringLiteral( "ST_NotSameAlignmentReason" ),
        QStringLiteral( "ST_NumBands" ),
        QStringLiteral( "ST_NumGeometries" ),
        QStringLiteral( "ST_NumInteriorRings" ),
        QStringLiteral( "ST_NumPatches" ),
        QStringLiteral( "ST_OffsetCurve" ),
        QStringLiteral( "ST_Orientation" ),
        QStringLiteral( "ST_Overlaps" ),
        QStringLiteral( "ST_PatchN" ),
        QStringLiteral( "ST_Perimeter" ),
        QStringLiteral( "ST_PixelAsCentroid" ),
        QStringLiteral( "ST_PixelAsCentroids" ),
        QStringLiteral( "ST_PixelAsPoint" ),
        QStringLiteral( "ST_PixelAsPoints" ),
        QStringLiteral( "ST_PixelAsPolygon" ),
        QStringLiteral( "ST_PixelAsPolygons" ),
        QStringLiteral( "ST_PixelHeight" ),
        QStringLiteral( "ST_PixelOfValue" ),
        QStringLiteral( "ST_PixelWidth" ),
        QStringLiteral( "ST_PointFromGeoHash" ),
        QStringLiteral( "ST_PointFromWKB" ),
        QStringLiteral( "ST_PointInsideCircle" ),
        QStringLiteral( "ST_PointN" ),
        QStringLiteral( "ST_PointOnSurface" ),
        QStringLiteral( "ST_Points" ),
        QStringLiteral( "ST_Polygon" ),
        QStringLiteral( "ST_Polygonize" ),
        QStringLiteral( "ST_Project" ),
        QStringLiteral( "ST_Quantile" ),
        QStringLiteral( "ST_Range4ma" ),
        QStringLiteral( "ST_RastFromHexWKB" ),
        QStringLiteral( "ST_RastFromWKB" ),
        QStringLiteral( "ST_RasterToWorldCoord" ),
        QStringLiteral( "ST_RasterToWorldCoordX" ),
        QStringLiteral( "ST_RasterToWorldCoordY" ),
        QStringLiteral( "ST_Reclass" ),
        QStringLiteral( "ST_ReducePrecision" ),
        QStringLiteral( "ST_Relate" ),
        QStringLiteral( "ST_RelateMatch" ),
        QStringLiteral( "ST_RemEdgeModFace" ),
        QStringLiteral( "ST_RemEdgeNewFace" ),
        QStringLiteral( "ST_RemovePoint" ),
        QStringLiteral( "ST_RemoveRepeatedPoints" ),
        QStringLiteral( "ST_Resample" ),
        QStringLiteral( "ST_Rescale" ),
        QStringLiteral( "ST_Resize" ),
        QStringLiteral( "ST_Reskew" ),
        QStringLiteral( "ST_Retile" ),
        QStringLiteral( "ST_Reverse" ),
        QStringLiteral( "ST_Rotate" ),
        QStringLiteral( "ST_RotateX" ),
        QStringLiteral( "ST_RotateY" ),
        QStringLiteral( "ST_RotateZ" ),
        QStringLiteral( "ST_Rotation" ),
        QStringLiteral( "ST_Roughness" ),
        QStringLiteral( "ST_SRID" ),
        QStringLiteral( "ST_SameAlignment" ),
        QStringLiteral( "ST_Scale" ),
        QStringLiteral( "ST_ScaleX" ),
        QStringLiteral( "ST_ScaleY" ),
        QStringLiteral( "ST_Segmentize" ),
        QStringLiteral( "ST_SetBandIndex" ),
        QStringLiteral( "ST_SetBandIsNoData" ),
        QStringLiteral( "ST_SetBandNoDataValue" ),
        QStringLiteral( "ST_SetBandPath" ),
        QStringLiteral( "ST_SetEffectiveArea" ),
        QStringLiteral( "ST_SetGeoReference" ),
        QStringLiteral( "ST_SetPoint" ),
        QStringLiteral( "ST_SetRotation" ),
        QStringLiteral( "ST_SetSRID" ),
        QStringLiteral( "ST_SetScale" ),
        QStringLiteral( "ST_SetSkew" ),
        QStringLiteral( "ST_SetUpperLeft" ),
        QStringLiteral( "ST_SetValue" ),
        QStringLiteral( "ST_SetValues" ),
        QStringLiteral( "ST_SharedPaths" ),
        QStringLiteral( "ST_ShiftLongitude" ),
        QStringLiteral( "ST_ShortestLine" ),
        QStringLiteral( "ST_Simplify" ),
        QStringLiteral( "ST_SimplifyPreserveTopology" ),
        QStringLiteral( "ST_SimplifyVW" ),
        QStringLiteral( "ST_SkewX" ),
        QStringLiteral( "ST_SkewY" ),
        QStringLiteral( "ST_Slope" ),
        QStringLiteral( "ST_Snap" ),
        QStringLiteral( "ST_SnapToGrid" ),
        QStringLiteral( "ST_Split" ),
        QStringLiteral( "ST_Square" ),
        QStringLiteral( "ST_SquareGrid" ),
        QStringLiteral( "ST_StartPoint" ),
        QStringLiteral( "ST_StdDev4ma" ),
        QStringLiteral( "ST_StraightSkeleton" ),
        QStringLiteral( "ST_Subdivide" ),
        QStringLiteral( "ST_Sum4ma" ),
        QStringLiteral( "ST_Summary" ),
        QStringLiteral( "ST_SummaryStats" ),
        QStringLiteral( "ST_SummaryStatsAgg" ),
        QStringLiteral( "ST_SwapOrdinates" ),
        QStringLiteral( "ST_SymDifference" ),
        QStringLiteral( "ST_TPI" ),
        QStringLiteral( "ST_TRI" ),
        QStringLiteral( "ST_Tesselate" ),  //#spellok
        QStringLiteral( "ST_Tile" ),
        QStringLiteral( "ST_TileEnvelope" ),
        QStringLiteral( "ST_Touches" ),
        QStringLiteral( "ST_TransScale" ),
        QStringLiteral( "ST_Transform" ),
        QStringLiteral( "ST_Translate" ),
        QStringLiteral( "ST_UnaryUnion" ),
        QStringLiteral( "ST_Union" ),
        QStringLiteral( "ST_UpperLeftX" ),
        QStringLiteral( "ST_UpperLeftY" ),
        QStringLiteral( "ST_Value" ),
        QStringLiteral( "ST_ValueCount" ),
        QStringLiteral( "ST_Volume" ),
        QStringLiteral( "ST_VoronoiLines" ),
        QStringLiteral( "ST_VoronoiPolygons" ),
        QStringLiteral( "ST_Width" ),
        QStringLiteral( "ST_Within" ),
        QStringLiteral( "ST_WorldToRasterCoord" ),
        QStringLiteral( "ST_WorldToRasterCoordX" ),
        QStringLiteral( "ST_WorldToRasterCoordY" ),
        QStringLiteral( "ST_WrapX" ),
        QStringLiteral( "ST_X" ),
        QStringLiteral( "ST_XMax" ),
        QStringLiteral( "ST_XMin" ),
        QStringLiteral( "ST_Y" ),
        QStringLiteral( "ST_YMax" ),
        QStringLiteral( "ST_YMin" ),
        QStringLiteral( "ST_Z" ),
        QStringLiteral( "ST_ZMax" ),
        QStringLiteral( "ST_ZMin" ),
        QStringLiteral( "ST_Zmflag" ),
        QStringLiteral( "Set_Geocode_Setting" ),
        QStringLiteral( "TopoElementArray_Agg" ),
        QStringLiteral( "TopoGeo_AddLineString" ),
        QStringLiteral( "TopoGeo_AddPoint" ),
        QStringLiteral( "TopoGeo_AddPolygon" ),
        QStringLiteral( "TopoGeom_addElement" ),
        QStringLiteral( "TopoGeom_remElement" ),
        QStringLiteral( "TopologySummary" ),
        QStringLiteral( "Topology_Load_Tiger" ),
        QStringLiteral( "UpdateGeometrySRID" ),
        QStringLiteral( "UpdateRasterSRID" ),
        QStringLiteral( "ValidateTopology" ),
        QStringLiteral( "box2d" ),
        QStringLiteral( "clearTopoGeom" ),
        QStringLiteral( "geometry_dump" ),
        QStringLiteral( "parse_address" ),
        QStringLiteral( "postgis.backend" ),
        QStringLiteral( "postgis.enable_outdb_rasters" ),
        QStringLiteral( "postgis.gdal_datapath" ),
        QStringLiteral( "postgis.gdal_enabled_drivers" ),
        QStringLiteral( "postgis_sfcgal_version" ),
        QStringLiteral( "standardize_address" ),
        QStringLiteral( "toTopoGeom" ),
        QStringLiteral( "|=|" ),
        QStringLiteral( "~" ),
        QStringLiteral( "~(box2df,box2df)" ),
        QStringLiteral( "~(box2df,geometry)" ),
        QStringLiteral( "~(geometry,box2df)" ),
        QStringLiteral( "~=" ),
      }
    }
  } );
}

QgsFields QgsPostgresProviderConnection::fields( const QString &schema, const QString &tableName ) const
{
  // Try the base implementation first and fall back to a more complex approach for the
  // few PG-specific corner cases that do not work with the base implementation.
  try
  {
    return QgsAbstractDatabaseProviderConnection::fields( schema, tableName );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    // This table might expose multiple geometry columns (different geom type or SRID)
    // but we are only interested in fields here, so let's pick the first one.
    TableProperty tableInfo { table( schema, tableName ) };
    try
    {
      QgsDataSourceUri tUri { tableUri( schema, tableName ) };

      if ( tableInfo.geometryColumnTypes().count( ) > 1 )
      {
        const auto geomColTypes( tableInfo.geometryColumnTypes() );
        TableProperty::GeometryColumnType geomCol { geomColTypes.first() };
        tUri.setGeometryColumn( tableInfo.geometryColumn() );
        tUri.setWkbType( geomCol.wkbType );
        tUri.setSrid( QString::number( geomCol.crs.postgisSrid() ) );
      }

      if ( tableInfo.primaryKeyColumns().count() > 0 )
      {
        const auto constPkCols( tableInfo.primaryKeyColumns() );
        tUri.setKeyColumn( constPkCols.first() );
      }

      tUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), QLatin1String( "0" ) );
      QgsVectorLayer::LayerOptions options { false, true };
      options.skipCrsValidation = true;

      QgsVectorLayer vl { tUri.uri(), QStringLiteral( "temp_layer" ), mProviderKey, options };

      if ( vl.isValid() )
      {
        return vl.fields();
      }
    }
    catch ( QgsProviderConnectionException & )
    {
      // fall-through
    }
    throw ex;
  }
}
