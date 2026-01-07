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

#include <chrono>

#include "qgsapplication.h"
#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprovider.h"
#include "qgspostgresprovidermetadatautils.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QIcon>
#include <QRegularExpression>

extern "C"
{
#include <libpq-fe.h>
}

// From configuration
const QStringList QgsPostgresProviderConnection::CONFIGURATION_PARAMETERS = {
  u"publicOnly"_s,
  u"geometryColumnsOnly"_s,
  u"dontResolveType"_s,
  u"allowGeometrylessTables"_s,
  u"saveUsername"_s,
  u"savePassword"_s,
  u"estimatedMetadata"_s,
  u"projectsInDatabase"_s,
  u"metadataInDatabase"_s,
  u"session_role"_s,
  u"allowRasterOverviewTables"_s,
  u"schema"_s,
};

const QString QgsPostgresProviderConnection::SETTINGS_BASE_KEY = u"/PostgreSQL/connections/"_s;


QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = u"postgres"_s;
  // Remove the sql and table empty parts
  const thread_local QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
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

QgsPostgresProviderConnection::QgsPostgresProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( QgsPostgresConn::connectionInfo( uri, false ), configuration )
{
  mProviderKey = u"postgres"_s;
  setDefaultCapabilities();
}

void QgsPostgresProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities = {
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
    Capability::AddField,
    Capability::RenameField,
    Capability::MoveTableToSchema
  };
  mGeometryColumnCapabilities = {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::SinglePoint,
    GeometryColumnCapability::SingleLineString,
    GeometryColumnCapability::SinglePolygon,
    GeometryColumnCapability::Curves,
    GeometryColumnCapability::PolyhedralSurfaces
  };
  mSqlLayerDefinitionCapabilities = {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds,
  };

  mCapabilities2 |= Qgis::DatabaseProviderConnectionCapability2::SetFieldComment
                    | Qgis::DatabaseProviderConnectionCapability2::SetTableComment;

  // see https://www.postgresql.org/docs/current/ddl-system-columns.html
  mIllegalFieldNames = {
    u"tableoid"_s,
    u"xmin"_s,
    u"cmin"_s,
    u"xmax"_s,
    u"cmax"_s,
    u"ctid"_s,

  };
}

void QgsPostgresProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  executeSqlPrivate( u"DROP TABLE %1.%2"_s
                       .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( name ) ) );
}

void QgsPostgresProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );

  QgsDataSourceUri newUri { uri() };
  newUri.setSchema( schema );
  newUri.setTable( name );
  // Set geometry column if it's not aspatial
  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry )
  {
    newUri.setGeometryColumn( options->value( u"geometryColumn"_s, u"geom"_s ).toString() );
  }

  QMap<int, int> map;
  QString errCause;
  QString createdLayerUri;
  Qgis::VectorExportResult res = QgsPostgresProvider::createEmptyLayer(
    newUri.uri(),
    fields,
    wkbType,
    srs,
    overwrite,
    &map,
    createdLayerUri,
    &errCause,
    options
  );
  if ( res != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

QString QgsPostgresProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  QgsDataSourceUri destUri( uri() );

  destUri.setTable( options.layerName );
  destUri.setSchema( options.schema );
  destUri.setGeometryColumn( options.wkbType != Qgis::WkbType::NoGeometry ? ( options.geometryColumn.isEmpty() ? u"geom"_s : options.geometryColumn ) : QString() );
  if ( !options.primaryKeyColumns.isEmpty() )
    destUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );

  providerOptions.clear();
  return destUri.uri( false );
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
  executeSqlPrivate( u"ALTER TABLE %1.%2 RENAME TO %3"_s
                       .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( name ), QgsPostgresConn::quotedIdentifier( newName ) ) );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsPostgresProviderConnection::tablesPrivate( const QString &schema, const QString &table, const TableFlags &flags, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  QList<QgsPostgresProviderConnection::TableProperty> tables;
  QString errCause;
  // TODO: set flags from the connection if flags argument is 0
  const QgsDataSourceUri dsUri { uri() };
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( dsUri, false ), -1, false, feedback );
  if ( feedback && feedback->isCanceled() )
    return {};

  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    bool ok { false };
    QVector<QgsPostgresLayerProperty> properties;
    const bool aspatial { !flags || flags.testFlag( TableFlag::Aspatial ) };
    if ( !table.isEmpty() )
    {
      QgsPostgresLayerProperty property;
      ok = conn->supportedLayer( property, schema, table );
      if ( ok )
      {
        properties.push_back( property );
      }
    }
    else
    {
      ok = conn->supportedLayers( properties, false, aspatial, false, schema );
    }

    if ( !ok )
    {
      if ( !table.isEmpty() )
      {
        errCause = QObject::tr( "Could not retrieve table '%2' from %1" ).arg( uri(), table );
      }
      else
      {
        errCause = QObject::tr( "Could not retrieve tables: %1" ).arg( uri() );
      }
    }
    else
    {
      bool dontResolveType = configuration().value( u"dontResolveType"_s, false ).toBool();
      bool useEstimatedMetadata = configuration().value( u"estimatedMetadata"_s, false ).toBool();

      // Cannot be const:
      for ( auto &pr : properties )
      {
        // Classify
        TableFlags prFlags;
        if ( pr.relKind == Qgis::PostgresRelKind::View || pr.relKind == Qgis::PostgresRelKind::MaterializedView )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::View );
        }
        if ( pr.relKind == Qgis::PostgresRelKind::MaterializedView )
        {
          prFlags.setFlag( QgsPostgresProviderConnection::TableFlag::MaterializedView );
        }
        if ( pr.relKind == Qgis::PostgresRelKind::ForeignTable )
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
        if ( !flags || ( prFlags & flags ) )
        {
          // retrieve layer types if needed
          if ( !dontResolveType && ( !pr.geometryColName.isNull() && ( pr.types.value( 0, Qgis::WkbType::Unknown ) == Qgis::WkbType::Unknown || pr.srids.value( 0, std::numeric_limits<int>::min() ) == std::numeric_limits<int>::min() ) ) )
          {
            conn->retrieveLayerTypes( pr, useEstimatedMetadata, feedback );
          }
          QgsPostgresProviderConnection::TableProperty property;
          property.setFlags( prFlags );
          for ( int i = 0; i < std::min( pr.types.size(), pr.srids.size() ); i++ )
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
          if ( pr.relKind == Qgis::PostgresRelKind::View
               || pr.relKind == Qgis::PostgresRelKind::MaterializedView
               || pr.relKind == Qgis::PostgresRelKind::ForeignTable )
          {
            // Set the candidates
            property.setPrimaryKeyColumns( pr.pkCols );
          }
          else // Fetch and set the real pks
          {
            try
            {
              const QList<QVariantList> pks = executeSqlPrivate( QStringLiteral( R"(
              WITH pkrelid AS (
              SELECT indexrelid AS idxri FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique)
                ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1)
              SELECT attname FROM pg_index,pg_attribute, pkrelid
              WHERE indexrelid=pkrelid.idxri AND indrelid=attrelid AND pg_attribute.attnum=any(pg_index.indkey);
             )" )
                                                                   .arg( QgsPostgresConn::quotedValue( QString( QgsPostgresConn::quotedIdentifier( pr.schemaName ) + "." + QgsPostgresConn::quotedIdentifier( pr.tableName ) ) ) ),
                                                                 false );
              QStringList pkNames;
              for ( const QVariantList &pk : std::as_const( pks ) )
              {
                pkNames.push_back( pk.first().toString() );
              }
              property.setPrimaryKeyColumns( pkNames );
            }
            catch ( const QgsProviderConnectionException &ex )
            {
              QgsDebugError( u"Error retrieving primary keys: %1"_s.arg( ex.what() ) );
            }
          }

          tables.push_back( property );
        }
      }
    }
    QgsPostgresConnPool::instance()->releaseConnection( conn );
  }
  if ( !errCause.isEmpty() )
  {
    throw QgsProviderConnectionException( errCause );
  }
  return tables;
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
  executeSqlPrivate( u"CREATE SCHEMA %1"_s
                       .arg( QgsPostgresConn::quotedIdentifier( name ) ) );
}

void QgsPostgresProviderConnection::dropSchema( const QString &name, bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( u"DROP SCHEMA %1 %2"_s
                       .arg( QgsPostgresConn::quotedIdentifier( name ), force ? u"CASCADE"_s : QString() ) );
}

void QgsPostgresProviderConnection::renameSchema( const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameSchema );
  executeSqlPrivate( u"ALTER SCHEMA %1 RENAME TO %2"_s
                       .arg( QgsPostgresConn::quotedIdentifier( name ), QgsPostgresConn::quotedIdentifier( newName ) ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsPostgresProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return execSqlPrivate( sql, true, feedback );
}

QList<QVariantList> QgsPostgresProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolPostgresConn> pgconn ) const
{
  return execSqlPrivate( sql, resolveTypes, feedback, std::move( pgconn ) ).rows();
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsPostgresProviderConnection::execSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback, std::shared_ptr<QgsPoolPostgresConn> pgconn ) const
{
  if ( !pgconn )
  {
    pgconn = std::make_shared<QgsPoolPostgresConn>( QgsPostgresConn::connectionInfo( QgsDataSourceUri( uri() ), false ) );
  }

  std::shared_ptr<QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator> iterator = std::make_shared<QgsPostgresProviderResultIterator>( resolveTypes );
  QueryResult results( iterator );

  // Check feedback first!
  if ( feedback && feedback->isCanceled() )
  {
    return results;
  }

  QgsPostgresConn *conn = pgconn->get();

  if ( !conn )
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
      qtConnection = QObject::connect( feedback, &QgsFeedback::canceled, [&pgconn] {
        if ( pgconn )
          pgconn->get()->PQCancel();
      } );
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto res = std::make_unique<QgsPostgresResult>( conn->LoggedPQexec( "QgsPostgresProviderConnection", sql ) );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );

    if ( feedback )
    {
      QObject::disconnect( qtConnection );
    }

    QString errCause;
    if ( conn->PQstatus() != CONNECTION_OK || !res->result() )
    {
      errCause = QObject::tr( "Connection error: %1 returned %2 [%3]" )
                   .arg( sql )
                   .arg( conn->PQstatus() )
                   .arg( conn->PQerrorMessage() );
    }
    else
    {
      const QString err { conn->PQerrorMessage() };
      if ( !err.isEmpty() )
      {
        errCause = QObject::tr( "SQL error: %1 returned %2 [%3]" )
                     .arg( sql )
                     .arg( conn->PQstatus() )
                     .arg( err );
      }
    }

    if ( !errCause.isEmpty() )
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

        const QList<QVariantList> typesResolved( executeSqlPrivate( u"SELECT oid, typname FROM pg_type WHERE oid IN (%1)"_s.arg( oids.join( ',' ) ), false, nullptr, pgconn ) );
        QgsStringMap oidTypeMap;
        for ( const auto &typeRes : std::as_const( typesResolved ) )
        {
          const QString oid { typeRes.constLast().toString() };
          if ( !oidTypeMap.contains( oid ) )
          {
            oidTypeMap.insert( typeRes.constFirst().toString(), typeRes.constLast().toString() );
          }
        }

        for ( int rowIdx = 0; rowIdx < numFields; rowIdx++ )
        {
          static const QStringList intTypes = {
            u"oid"_s,
            u"int2"_s,
            u"int4"_s,
            u"int8"_s,
          };
          static const QStringList floatTypes = { u"float4"_s, u"float8"_s, u"numeric"_s };

          const QString typName { oidTypeMap[oids.at( rowIdx )] };
          QMetaType::Type vType { QMetaType::Type::QString };
          if ( floatTypes.contains( typName ) )
          {
            vType = QMetaType::Type::Double;
          }
          else if ( intTypes.contains( typName ) )
          {
            vType = QMetaType::Type::LongLong;
          }
          else if ( typName == "date"_L1 )
          {
            vType = QMetaType::Type::QDate;
          }
          else if ( typName.startsWith( "timestamp"_L1 ) )
          {
            vType = QMetaType::Type::QDateTime;
          }
          else if ( typName == "time"_L1 )
          {
            vType = QMetaType::Type::QTime;
          }
          else if ( typName == "bool"_L1 )
          {
            vType = QMetaType::Type::Bool;
          }
          else if ( typName == "char"_L1 )
          {
            vType = QMetaType::Type::QChar;
          }
          else
          {
            // Just a warning, usually ok
            QgsDebugMsgLevel( u"Unhandled PostgreSQL type %1, assuming string"_s.arg( typName ), 2 );
          }
          static_cast<QgsPostgresProviderResultIterator *>( iterator.get() )->typeMap[rowIdx] = vType;
        }
      }
    }
    if ( !errCause.isEmpty() )
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
      const QMetaType::Type vType { typeMap.value( colIdx, QMetaType::Type::QString ) };
      QVariant val = result->PQgetvalue( mRowIndex, colIdx );
      // Special case for bools: 'f' and 't'
      if ( vType == QMetaType::Type::Bool )
      {
        const QString boolStrVal { val.toString() };
        if ( !boolStrVal.isEmpty() )
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
  return result ? result->PQntuples() : static_cast<long long>( Qgis::FeatureCountState::UnknownCount );
}


void QgsPostgresProviderConnection::vacuum( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::Vacuum );
  if ( !schema.isEmpty() && !name.isEmpty() )
  {
    executeSqlPrivate( u"VACUUM FULL ANALYZE %1.%2"_s.arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( name ) ), false );
  }
  else
  {
    executeSqlPrivate( u"VACUUM FULL ANALYZE"_s );
  }
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

  const QString indexName = u"sidx_%1_%2"_s.arg( name, geometryColumnName );
  executeSqlPrivate( u"CREATE INDEX %1 ON %2.%3 USING GIST (%4);"_s.arg( QgsPostgresConn::quotedIdentifier( indexName ), QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( name ), QgsPostgresConn::quotedIdentifier( geometryColumnName ) ), false );
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
                                                              )""" )
                                                .arg(
                                                  QgsPostgresConn::quotedValue( schema ),
                                                  QgsPostgresConn::quotedValue( name ),
                                                  QgsPostgresConn::quotedValue( geometryColumn )
                                                ) );
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
                                                                )""" )
                                                .arg(
                                                  QgsPostgresConn::quotedValue( schema ),
                                                  QgsPostgresConn::quotedValue( name ),
                                                  QgsPostgresConn::quotedValue( geometryColumn )
                                                ) );
  if ( res.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "No spatial index exists for %1.%2" ).arg( schema, name ) );

  const QString indexName = res.at( 0 ).at( 0 ).toString();

  executeSqlPrivate( u"DROP INDEX %1.%2"_s.arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( indexName ) ), false );
}

void QgsPostgresProviderConnection::setFieldComment( const QString &fieldName, const QString &schema, const QString &tableName, const QString &comment ) const
{
  executeSqlPrivate( u"COMMENT ON COLUMN %1.%2.%3 IS %4;"_s
                       .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( tableName ), QgsPostgresConn::quotedIdentifier( fieldName ), QgsPostgresConn::quotedValue( comment ) ) );
}

void QgsPostgresProviderConnection::setTableComment( const QString &schema, const QString &tableName, const QString &comment ) const
{
  executeSqlPrivate( u"COMMENT ON TABLE %1.%2 IS %3;"_s
                       .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( tableName ), QgsPostgresConn::quotedValue( comment ) ) );
}

QList<QgsPostgresProviderConnection::TableProperty> QgsPostgresProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{
  return tablesPrivate( schema, QString(), flags, feedback );
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsPostgresProviderConnection::table( const QString &schema, const QString &table, QgsFeedback *feedback ) const
{
  const QList<QgsPostgresProviderConnection::TableProperty> properties { tablesPrivate( schema, table, TableFlags(), feedback ) };
  if ( !properties.empty() )
  {
    return properties.first();
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "Table '%1' was not found in schema '%2'" )
                                            .arg( table, schema ) );
  }
}

QStringList QgsPostgresProviderConnection::schemas() const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;
  QString errCause;
  const QgsDataSourceUri dsUri { uri() };
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( dsUri, false ) );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection failed: %1" ).arg( uri() );
  }
  else
  {
    QList<QgsPostgresSchemaProperty> schemaProperties;
    bool ok = conn->getSchemas( schemaProperties );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    if ( !ok )
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
  if ( !errCause.isEmpty() )
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
  settings.setValue( "host", dsUri.host() );
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
    else
    {
      settings.remove( p );
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
  return QgsApplication::getThemeIcon( u"mIconPostgis.svg"_s );
}


QList<QgsVectorDataProvider::NativeType> QgsPostgresProviderConnection::nativeTypes() const
{
  QList<QgsVectorDataProvider::NativeType> types;
  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( QgsPostgresConn::connectionInfo( QgsDataSourceUri { uri() }, false ) );
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
  options.sql = trimmedTable.startsWith( '(' ) ? trimmedTable.mid( 1 ).chopped( 1 ) : u"SELECT * FROM %1"_s.arg( tUri.quotedTablename() );
  return options;
}

QList<QgsLayerMetadataProviderResult> QgsPostgresProviderConnection::searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const
{
  return QgsPostgresProviderMetadataUtils::searchLayerMetadata( searchContext, uri(), searchString, geographicExtent, feedback );
}

Qgis::DatabaseProviderTableImportCapabilities QgsPostgresProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName | Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName;
}

QString QgsPostgresProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"id"_s;
}

QgsVectorLayer *QgsPostgresProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions &options ) const
{
  // Precondition
  if ( options.sql.isEmpty() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not create a SQL vector layer: SQL expression is empty." ) );
  }

  QgsDataSourceUri tUri( uri() );

  tUri.setSql( options.filter );
  tUri.disableSelectAtId( options.disableSelectAtId );

  if ( !options.primaryKeyColumns.isEmpty() )
  {
    tUri.setKeyColumn( options.primaryKeyColumns.join( ',' ) );
    tUri.setTable( u"(%1)"_s.arg( sanitizeSqlForQueryLayer( options.sql ) ) );
  }
  else
  {
    int pkId { 0 };
    while ( options.sql.contains( u"_uid%1_"_s.arg( pkId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      pkId++;
    }
    tUri.setKeyColumn( u"_uid%1_"_s.arg( pkId ) );

    int sqlId { 0 };
    while ( options.sql.contains( u"_subq_%1_"_s.arg( sqlId ), Qt::CaseSensitivity::CaseInsensitive ) )
    {
      sqlId++;
    }
    tUri.setTable( u"(SELECT row_number() over () AS _uid%1_, * FROM (%2\n) AS _subq_%3_\n)"_s.arg( QString::number( pkId ), sanitizeSqlForQueryLayer( options.sql ), QString::number( sqlId ) ) );
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer { tUri.uri( false ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey(), vectorLayerOptions };
}

QMultiMap<Qgis::SqlKeywordCategory, QStringList> QgsPostgresProviderConnection::sqlDictionary()
{
  return QgsAbstractDatabaseProviderConnection::sqlDictionary().unite(
    { { Qgis::SqlKeywordCategory::Keyword,
        {
          u"absolute"_s,
          u"action"_s,
          u"add"_s,
          u"admin"_s,
          u"after"_s,
          u"aggregate"_s,
          u"alias"_s,
          u"all"_s,
          u"allocate"_s,
          u"alter"_s,
          u"analyse"_s,
          u"analyze"_s,
          u"and"_s,
          u"any"_s,
          u"are"_s,
          u"array"_s,
          u"as"_s,
          u"asc"_s,
          u"asensitive"_s,
          u"assertion"_s,
          u"asymmetric"_s,
          u"at"_s,
          u"atomic"_s,
          u"authorization"_s,
          u"avg"_s,
          u"before"_s,
          u"begin"_s,
          u"between"_s,
          u"bigint"_s,
          u"binary"_s,
          u"bit"_s,
          u"bit_length"_s,
          u"blob"_s,
          u"boolean"_s,
          u"both"_s,
          u"breadth"_s,
          u"by"_s,
          u"call"_s,
          u"called"_s,
          u"cardinality"_s,
          u"cascade"_s,
          u"cascaded"_s,
          u"case"_s,
          u"cast"_s,
          u"catalog"_s,
          u"ceil"_s,
          u"ceiling"_s,
          u"char"_s,
          u"character"_s,
          u"character_length"_s,
          u"char_length"_s,
          u"check"_s,
          u"class"_s,
          u"clob"_s,
          u"close"_s,
          u"coalesce"_s,
          u"collate"_s,
          u"collation"_s,
          u"collect"_s,
          u"column"_s,
          u"commit"_s,
          u"completion"_s,
          u"condition"_s,
          u"connect"_s,
          u"connection"_s,
          u"constraint"_s,
          u"constraints"_s,
          u"constructor"_s,
          u"continue"_s,
          u"convert"_s,
          u"corr"_s,
          u"corresponding"_s,
          u"count"_s,
          u"covar_pop"_s,
          u"covar_samp"_s,
          u"create"_s,
          u"cross"_s,
          u"cube"_s,
          u"cume_dist"_s,
          u"current"_s,
          u"current_date"_s,
          u"current_default_transform_group"_s,
          u"current_path"_s,
          u"current_role"_s,
          u"current_time"_s,
          u"current_timestamp"_s,
          u"current_transform_group_for_type"_s,
          u"current_user"_s,
          u"cursor"_s,
          u"cycle"_s,
          u"data"_s,
          u"date"_s,
          u"day"_s,
          u"deallocate"_s,
          u"dec"_s,
          u"decimal"_s,
          u"declare"_s,
          u"default"_s,
          u"deferrable"_s,
          u"deferred"_s,
          u"delete"_s,
          u"dense_rank"_s,
          u"depth"_s,
          u"deref"_s,
          u"desc"_s,
          u"describe"_s,
          u"descriptor"_s,
          u"destroy"_s,
          u"destructor"_s,
          u"deterministic"_s,
          u"diagnostics"_s,
          u"dictionary"_s,
          u"disconnect"_s,
          u"distinct"_s,
          u"do"_s,
          u"domain"_s,
          u"double"_s,
          u"drop"_s,
          u"dynamic"_s,
          u"each"_s,
          u"element"_s,
          u"else"_s,
          u"end"_s,
          u"end-exec"_s,
          u"equals"_s,
          u"escape"_s,
          u"every"_s,
          u"except"_s,
          u"exception"_s,
          u"exec"_s,
          u"execute"_s,
          u"exists"_s,
          u"exp"_s,
          u"external"_s,
          u"extract"_s,
          u"false"_s,
          u"fetch"_s,
          u"filter"_s,
          u"first"_s,
          u"float"_s,
          u"floor"_s,
          u"for"_s,
          u"foreign"_s,
          u"found"_s,
          u"free"_s,
          u"freeze"_s,
          u"from"_s,
          u"full"_s,
          u"function"_s,
          u"fusion"_s,
          u"general"_s,
          u"get"_s,
          u"global"_s,
          u"go"_s,
          u"goto"_s,
          u"grant"_s,
          u"group"_s,
          u"grouping"_s,
          u"having"_s,
          u"hold"_s,
          u"host"_s,
          u"hour"_s,
          u"identity"_s,
          u"ignore"_s,
          u"ilike"_s,
          u"immediate"_s,
          u"in"_s,
          u"indicator"_s,
          u"initialize"_s,
          u"initially"_s,
          u"inner"_s,
          u"inout"_s,
          u"input"_s,
          u"insensitive"_s,
          u"insert"_s,
          u"int"_s,
          u"integer"_s,
          u"intersect"_s,
          u"intersection"_s,
          u"interval"_s,
          u"into"_s,
          u"is"_s,
          u"isnull"_s,
          u"isolation"_s,
          u"iterate"_s,
          u"join"_s,
          u"key"_s,
          u"language"_s,
          u"large"_s,
          u"last"_s,
          u"lateral"_s,
          u"leading"_s,
          u"left"_s,
          u"less"_s,
          u"level"_s,
          u"like"_s,
          u"limit"_s,
          u"ln"_s,
          u"local"_s,
          u"localtime"_s,
          u"localtimestamp"_s,
          u"locator"_s,
          u"lower"_s,
          u"map"_s,
          u"match"_s,
          u"max"_s,
          u"member"_s,
          u"merge"_s,
          u"method"_s,
          u"min"_s,
          u"minute"_s,
          u"mod"_s,
          u"modifies"_s,
          u"modify"_s,
          u"module"_s,
          u"month"_s,
          u"multiset"_s,
          u"names"_s,
          u"national"_s,
          u"natural"_s,
          u"nchar"_s,
          u"nclob"_s,
          u"new"_s,
          u"next"_s,
          u"no"_s,
          u"none"_s,
          u"normalize"_s,
          u"not"_s,
          u"notnull"_s,
          u"null"_s,
          u"nullif"_s,
          u"numeric"_s,
          u"object"_s,
          u"octet_length"_s,
          u"of"_s,
          u"off"_s,
          u"offset"_s,
          u"old"_s,
          u"on"_s,
          u"only"_s,
          u"open"_s,
          u"operation"_s,
          u"option"_s,
          u"or"_s,
          u"order"_s,
          u"ordinality"_s,
          u"out"_s,
          u"outer"_s,
          u"output"_s,
          u"over"_s,
          u"overlaps"_s,
          u"overlay"_s,
          u"pad"_s,
          u"parameter"_s,
          u"parameters"_s,
          u"partial"_s,
          u"partition"_s,
          u"path"_s,
          u"percentile_cont"_s,
          u"percentile_disc"_s,
          u"percent_rank"_s,
          u"placing"_s,
          u"position"_s,
          u"postfix"_s,
          u"power"_s,
          u"precision"_s,
          u"prefix"_s,
          u"preorder"_s,
          u"prepare"_s,
          u"preserve"_s,
          u"primary"_s,
          u"prior"_s,
          u"privileges"_s,
          u"procedure"_s,
          u"public"_s,
          u"range"_s,
          u"rank"_s,
          u"read"_s,
          u"reads"_s,
          u"real"_s,
          u"recursive"_s,
          u"ref"_s,
          u"references"_s,
          u"referencing"_s,
          u"regr_avgx"_s,
          u"regr_avgy"_s,
          u"regr_count"_s,
          u"regr_intercept"_s,
          u"regr_r2"_s,
          u"regr_slope"_s,
          u"regr_sxx"_s,
          u"regr_sxy"_s,
          u"regr_syy"_s,
          u"relative"_s,
          u"release"_s,
          u"restrict"_s,
          u"result"_s,
          u"return"_s,
          u"returning"_s,
          u"returns"_s,
          u"revoke"_s,
          u"right"_s,
          u"role"_s,
          u"rollback"_s,
          u"rollup"_s,
          u"routine"_s,
          u"row"_s,
          u"row_number"_s,
          u"rows"_s,
          u"savepoint"_s,
          u"schema"_s,
          u"scope"_s,
          u"scroll"_s,
          u"search"_s,
          u"second"_s,
          u"section"_s,
          u"select"_s,
          u"sensitive"_s,
          u"sequence"_s,
          u"session"_s,
          u"session_user"_s,
          u"set"_s,
          u"sets"_s,
          u"similar"_s,
          u"size"_s,
          u"smallint"_s,
          u"some"_s,
          u"space"_s,
          u"specific"_s,
          u"specifictype"_s,
          u"sql"_s,
          u"sqlcode"_s,
          u"sqlerror"_s,
          u"sqlexception"_s,
          u"sqlstate"_s,
          u"sqlwarning"_s,
          u"sqrt"_s,
          u"start"_s,
          u"state"_s,
          u"statement"_s,
          u"static"_s,
          u"stddev_pop"_s,
          u"stddev_samp"_s,
          u"structure"_s,
          u"submultiset"_s,
          u"substring"_s,
          u"sum"_s,
          u"symmetric"_s,
          u"system"_s,
          u"system_user"_s,
          u"table"_s,
          u"tablesample"_s,
          u"temporary"_s,
          u"terminate"_s,
          u"than"_s,
          u"then"_s,
          u"time"_s,
          u"timestamp"_s,
          u"timezone_hour"_s,
          u"timezone_minute"_s,
          u"to"_s,
          u"trailing"_s,
          u"transaction"_s,
          u"translate"_s,
          u"translation"_s,
          u"treat"_s,
          u"trigger"_s,
          u"trim"_s,
          u"true"_s,
          u"uescape"_s,
          u"under"_s,
          u"union"_s,
          u"unique"_s,
          u"unknown"_s,
          u"unnest"_s,
          u"update"_s,
          u"upper"_s,
          u"usage"_s,
          u"user"_s,
          u"using"_s,
          u"value"_s,
          u"values"_s,
          u"varchar"_s,
          u"variable"_s,
          u"var_pop"_s,
          u"var_samp"_s,
          u"varying"_s,
          u"verbose"_s,
          u"view"_s,
          u"when"_s,
          u"whenever"_s,
          u"where"_s,
          u"width_bucket"_s,
          u"window"_s,
          u"with"_s,
          u"within"_s,
          u"without"_s,
          u"work"_s,
          u"write"_s,
          u"xml"_s,
          u"xmlagg"_s,
          u"xmlattributes"_s,
          u"xmlbinary"_s,
          u"xmlcomment"_s,
          u"xmlconcat"_s,
          u"xmlelement"_s,
          u"xmlforest"_s,
          u"xmlnamespaces"_s,
          u"xmlparse"_s,
          u"xmlpi"_s,
          u"xmlroot"_s,
          u"xmlserialize"_s,
          u"year"_s,
          u"zone"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Aggregate,
        {
          u"Max"_s,
          u"Min"_s,
          u"Avg"_s,
          u"Count"_s,
          u"Sum"_s,
          u"Group_Concat"_s,
          u"Total"_s,
          u"Var_Pop"_s,
          u"Var_Samp"_s,
          u"StdDev_Pop"_s,
          u"StdDev_Samp"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Math,
        {
          u"Abs"_s,
          u"ACos"_s,
          u"ASin"_s,
          u"ATan"_s,
          u"Cos"_s,
          u"Cot"_s,
          u"Degrees"_s,
          u"Exp"_s,
          u"Floor"_s,
          u"Log"_s,
          u"Log2"_s,

          u"Log10"_s,
          u"Pi"_s,
          u"Radians"_s,
          u"Round"_s,
          u"Sign"_s,
          u"Sin"_s,
          u"Sqrt"_s,
          u"StdDev_Pop"_s,
          u"StdDev_Samp"_s,
          u"Tan"_s,
          u"Var_Pop"_s,
          u"Var_Samp"_s,
        }
      },
      { Qgis::SqlKeywordCategory::Geospatial,
        {
          // List from:
          // import requests, re, html;
          // result = requests.get('https://postgis.net/docs/PostGIS_Special_Functions_Index.html')
          // m = re.findall('<a class="link".*?title="(.*?)">.*?</li>', result.content.replace(b'\xc2', b'').replace(b'\xa0', b'').decode('utf8'), re.MULTILINE)
          // for f_name in sorted(set(m)):
          //     print(f'              u"{html.unescape(f_name)}"_s,')

          u"&&"_s,
          u"&&&"_s,
          u"&&&(geometry,gidx)"_s,
          u"&&&(gidx,geometry)"_s,
          u"&&&(gidx,gidx)"_s,
          u"&&(box2df,box2df)"_s,
          u"&&(box2df,geometry)"_s,
          u"&&(geometry,box2df)"_s,
          u"&>"_s,
          u"&<"_s,
          u"&<|"_s,
          u"<#>"_s,
          u"<<#>>"_s,
          u"<<->>"_s,
          u"<->"_s,
          u"="_s,
          u"@"_s,
          u"@(box2df,box2df)"_s,
          u"@(box2df,geometry)"_s,
          u"@(geometry,box2df)"_s,
          u"AddEdge"_s,
          u"AddFace"_s,
          u"AddGeometryColumn"_s,
          u"AddNode"_s,
          u"AddOverviewConstraints"_s,
          u"AddRasterConstraints"_s,
          u"AsGML"_s,
          u"AsTopoJSON"_s,
          u"Box2D"_s,
          u"Box3D"_s,
          u"CopyTopology"_s,
          u"DropGeometryColumn"_s,
          u"DropOverviewConstraints"_s,
          u"DropRasterConstraints"_s,
          u"Drop_Indexes_Generate_Script"_s,
          u"Drop_Nation_Tables_Generate_Script"_s,
          u"Drop_State_Tables_Generate_Script"_s,
          u"Equals"_s,
          u"Geocode"_s,
          u"Geocode_Intersection"_s,
          u"GeometryType"_s,
          u"GetEdgeByPoint"_s,
          u"GetFaceByPoint"_s,
          u"GetNodeByPoint"_s,
          u"GetNodeEdges"_s,
          u"GetRingEdges"_s,
          u"GetTopoGeomElements"_s,
          u"GetTopologySRID"_s,
          u"Get_Geocode_Setting"_s,
          u"Get_Tract"_s,
          u"Install_Missing_Indexes"_s,
          u"Intersects"_s,
          u"Loader_Generate_Census_Script"_s,
          u"Loader_Generate_Nation_Script"_s,
          u"Loader_Generate_Script"_s,
          u"Missing_Indexes_Generate_Script"_s,
          u"Normalize_Address"_s,
          u"Pagc_Normalize_Address"_s,
          u"Polygonize"_s,
          u"Populate_Geometry_Columns"_s,
          u"Populate_Topology_Layer"_s,
          u"PostGIS_AddBBox"_s,
          u"PostGIS_DropBBox"_s,
          u"PostGIS_Extensions_Upgrade"_s,
          u"PostGIS_HasBBox"_s,
          u"PostGIS_LibXML_Version"_s,
          u"Reverse_Geocode"_s,
          u"ST_3DArea"_s,
          u"ST_3DClosestPoint"_s,
          u"ST_3DDFullyWithin"_s,
          u"ST_3DDWithin"_s,
          u"ST_3DDifference"_s,
          u"ST_3DDistance"_s,
          u"ST_3DExtent"_s,
          u"ST_3DIntersection"_s,
          u"ST_3DIntersects"_s,
          u"ST_3DLength"_s,
          u"ST_3DLineInterpolatePoint"_s,
          u"ST_3DLongestLine"_s,
          u"ST_3DMakeBox"_s,
          u"ST_3DMaxDistance"_s,
          u"ST_3DPerimeter"_s,
          u"ST_3DShortestLine"_s,
          u"ST_3DUnion"_s,
          u"ST_AddBand"_s,
          u"ST_AddEdgeModFace"_s,
          u"ST_AddEdgeNewFaces"_s,
          u"ST_AddMeasure"_s,
          u"ST_AddPoint"_s,
          u"ST_Affine"_s,
          u"ST_Angle"_s,
          u"ST_ApproximateMedialAxis"_s,
          u"ST_Area"_s,
          u"ST_AsBinary"_s,
          u"ST_AsBinary/ST_AsWKB"_s,
          u"ST_AsEWKB"_s,
          u"ST_AsEWKT"_s,
          u"ST_AsEncodedPolyline"_s,
          u"ST_AsGDALRaster"_s,
          u"ST_AsGML"_s,
          u"ST_AsGeoJSON"_s,
          u"ST_AsGeobuf"_s,
          u"ST_AsHEXEWKB"_s,
          u"ST_AsHexWKB"_s,
          u"ST_AsJPEG"_s,
          u"ST_AsKML"_s,
          u"ST_AsLatLonText"_s,
          u"ST_AsMVT"_s,
          u"ST_AsMVTGeom"_s,
          u"ST_AsPNG"_s,
          u"ST_AsRaster"_s,
          u"ST_AsSVG"_s,
          u"ST_AsTIFF"_s,
          u"ST_AsTWKB"_s,
          u"ST_AsText"_s,
          u"ST_AsX3D"_s,
          u"ST_Aspect"_s,
          u"ST_Azimuth"_s,
          u"ST_Band"_s,
          u"ST_BandFileSize"_s,
          u"ST_BandFileTimestamp"_s,
          u"ST_BandIsNoData"_s,
          u"ST_BandMetaData"_s,
          u"ST_BandNoDataValue"_s,
          u"ST_BandPath"_s,
          u"ST_BandPixelType"_s,
          u"ST_Boundary"_s,
          u"ST_BoundingDiagonal"_s,
          u"ST_Box2dFromGeoHash"_s,
          u"ST_Buffer"_s,
          u"ST_CPAWithin"_s,
          u"ST_Centroid"_s,
          u"ST_ChaikinSmoothing"_s,
          u"ST_Clip"_s,
          u"ST_ClipByBox2D"_s,
          u"ST_ClosestPoint"_s,
          u"ST_ClosestPointOfApproach"_s,
          u"ST_ClusterDBSCAN"_s,
          u"ST_ClusterIntersecting"_s,
          u"ST_ClusterKMeans"_s,
          u"ST_ClusterWithin"_s,
          u"ST_Collect"_s,
          u"ST_CollectionExtract"_s,
          u"ST_CollectionHomogenize"_s,
          u"ST_ColorMap"_s,
          u"ST_ConcaveHull"_s,
          u"ST_ConstrainedDelaunayTriangles"_s,
          u"ST_Contains"_s,
          u"ST_ContainsProperly"_s,
          u"ST_ConvexHull"_s,
          u"ST_CoordDim"_s,
          u"ST_Count"_s,
          u"ST_CountAgg"_s,
          u"ST_CoveredBy"_s,
          u"ST_Covers"_s,
          u"ST_CreateOverview"_s,
          u"ST_CreateTopoGeo"_s,
          u"ST_Crosses"_s,
          u"ST_CurveToLine"_s,
          u"ST_DFullyWithin"_s,
          u"ST_DWithin"_s,
          u"ST_DelaunayTriangles"_s,
          u"ST_Difference"_s,
          u"ST_Dimension"_s,
          u"ST_Disjoint"_s,
          u"ST_Distance"_s,
          u"ST_DistanceCPA"_s,
          u"ST_DistanceSphere"_s,
          u"ST_DistanceSpheroid"_s,
          u"ST_Distinct4ma"_s,
          u"ST_Dump"_s,
          u"ST_DumpAsPolygons"_s,
          u"ST_DumpPoints"_s,
          u"ST_DumpRings"_s,
          u"ST_DumpValues"_s,
          u"ST_EndPoint"_s,
          u"ST_Envelope"_s,
          u"ST_Equals"_s,
          u"ST_EstimatedExtent"_s,
          u"ST_Expand"_s,
          u"ST_Extent"_s,
          u"ST_ExteriorRing"_s,
          u"ST_Extrude"_s,
          u"ST_FilterByM"_s,
          u"ST_FlipCoordinates"_s,
          u"ST_Force2D"_s,
          u"ST_Force3D"_s,
          u"ST_Force3DM"_s,
          u"ST_Force3DZ"_s,
          u"ST_Force4D"_s,
          u"ST_ForceCollection"_s,
          u"ST_ForceCurve"_s,
          u"ST_ForceLHR"_s,
          u"ST_ForcePolygonCCW"_s,
          u"ST_ForcePolygonCW"_s,
          u"ST_ForceRHR"_s,
          u"ST_ForceSFS"_s,
          u"ST_FrechetDistance"_s,
          u"ST_FromGDALRaster"_s,
          u"ST_GDALDrivers"_s,
          u"ST_GMLToSQL"_s,
          u"ST_GeneratePoints"_s,
          u"ST_GeoHash"_s,
          u"ST_GeoReference"_s,
          u"ST_GeogFromText"_s,
          u"ST_GeogFromWKB"_s,
          u"ST_GeographyFromText"_s,
          u"ST_GeomFromEWKB"_s,
          u"ST_GeomFromEWKT"_s,
          u"ST_GeomFromGML"_s,
          u"ST_GeomFromGeoHash"_s,
          u"ST_GeomFromGeoJSON"_s,
          u"ST_GeomFromKML"_s,
          u"ST_GeomFromText"_s,
          u"ST_GeomFromWKB"_s,
          u"ST_GeometricMedian"_s,
          u"ST_GeometryN"_s,
          u"ST_GeometryType"_s,
          u"ST_GetFaceEdges"_s,
          u"ST_Grayscale"_s,
          u"ST_HasArc"_s,
          u"ST_HasNoBand"_s,
          u"ST_HausdorffDistance"_s,
          u"ST_Height"_s,
          u"ST_Hexagon"_s,
          u"ST_HexagonGrid"_s,
          u"ST_HillShade"_s,
          u"ST_Histogram"_s,
          u"ST_InteriorRingN"_s,
          u"ST_InterpolatePoint"_s,
          u"ST_Intersection"_s,
          u"ST_Intersects"_s,
          u"ST_InvDistWeight4ma"_s,
          u"ST_IsClosed"_s,
          u"ST_IsCollection"_s,
          u"ST_IsEmpty"_s,
          u"ST_IsPlanar"_s,
          u"ST_IsPolygonCCW"_s,
          u"ST_IsPolygonCW"_s,
          u"ST_IsSimple"_s,
          u"ST_IsSolid"_s,
          u"ST_IsValidDetail"_s,
          u"ST_IsValidReason"_s,
          u"ST_IsValidTrajectory"_s,
          u"ST_Length"_s,
          u"ST_LengthSpheroid"_s,
          u"ST_LineCrossingDirection"_s,
          u"ST_LineFromEncodedPolyline"_s,
          u"ST_LineFromMultiPoint"_s,
          u"ST_LineInterpolatePoint"_s,
          u"ST_LineInterpolatePoints"_s,
          u"ST_LineLocatePoint"_s,
          u"ST_LineSubstring"_s,
          u"ST_LineToCurve"_s,
          u"ST_LocateAlong"_s,
          u"ST_LocateBetween"_s,
          u"ST_LocateBetweenElevations"_s,
          u"ST_LongestLine"_s,
          u"ST_M"_s,
          u"ST_MakeBox2D"_s,
          u"ST_MakeEmptyCoverage"_s,
          u"ST_MakeEmptyRaster"_s,
          u"ST_MakeEnvelope"_s,
          u"ST_MakeLine"_s,
          u"ST_MakePoint"_s,
          u"ST_MakePolygon"_s,
          u"ST_MakeSolid"_s,
          u"ST_MakeValid"_s,
          u"ST_MapAlgebra (callback function version)"_s,
          u"ST_MapAlgebra (expression version)"_s,
          u"ST_MapAlgebraExpr"_s,
          u"ST_MapAlgebraFct"_s,
          u"ST_MapAlgebraFctNgb"_s,
          u"ST_Max4ma"_s,
          u"ST_MaxDistance"_s,
          u"ST_MaximumInscribedCircle"_s,
          u"ST_Mean4ma"_s,
          u"ST_MemSize"_s,
          u"ST_MemUnion"_s,
          u"ST_MetaData"_s,
          u"ST_Min4ma"_s,
          u"ST_MinConvexHull"_s,
          u"ST_MinDist4ma"_s,
          u"ST_MinimumBoundingCircle"_s,
          u"ST_MinimumClearance"_s,
          u"ST_MinimumClearanceLine"_s,
          u"ST_MinkowskiSum"_s,
          u"ST_ModEdgeHeal"_s,
          u"ST_ModEdgeSplit"_s,
          u"ST_NDims"_s,
          u"ST_NPoints"_s,
          u"ST_NRings"_s,
          u"ST_NearestValue"_s,
          u"ST_Neighborhood"_s,
          u"ST_NewEdgeHeal"_s,
          u"ST_Node"_s,
          u"ST_Normalize"_s,
          u"ST_NotSameAlignmentReason"_s,
          u"ST_NumBands"_s,
          u"ST_NumGeometries"_s,
          u"ST_NumInteriorRings"_s,
          u"ST_NumPatches"_s,
          u"ST_OffsetCurve"_s,
          u"ST_Orientation"_s,
          u"ST_Overlaps"_s,
          u"ST_PatchN"_s,
          u"ST_Perimeter"_s,
          u"ST_PixelAsCentroid"_s,
          u"ST_PixelAsCentroids"_s,
          u"ST_PixelAsPoint"_s,
          u"ST_PixelAsPoints"_s,
          u"ST_PixelAsPolygon"_s,
          u"ST_PixelAsPolygons"_s,
          u"ST_PixelHeight"_s,
          u"ST_PixelOfValue"_s,
          u"ST_PixelWidth"_s,
          u"ST_PointFromGeoHash"_s,
          u"ST_PointFromWKB"_s,
          u"ST_PointInsideCircle"_s,
          u"ST_PointN"_s,
          u"ST_PointOnSurface"_s,
          u"ST_Points"_s,
          u"ST_Polygon"_s,
          u"ST_Polygonize"_s,
          u"ST_Project"_s,
          u"ST_Quantile"_s,
          u"ST_Range4ma"_s,
          u"ST_RastFromHexWKB"_s,
          u"ST_RastFromWKB"_s,
          u"ST_RasterToWorldCoord"_s,
          u"ST_RasterToWorldCoordX"_s,
          u"ST_RasterToWorldCoordY"_s,
          u"ST_Reclass"_s,
          u"ST_ReducePrecision"_s,
          u"ST_Relate"_s,
          u"ST_RelateMatch"_s,
          u"ST_RemEdgeModFace"_s,
          u"ST_RemEdgeNewFace"_s,
          u"ST_RemovePoint"_s,
          u"ST_RemoveRepeatedPoints"_s,
          u"ST_Resample"_s,
          u"ST_Rescale"_s,
          u"ST_Resize"_s,
          u"ST_Reskew"_s,
          u"ST_Retile"_s,
          u"ST_Reverse"_s,
          u"ST_Rotate"_s,
          u"ST_RotateX"_s,
          u"ST_RotateY"_s,
          u"ST_RotateZ"_s,
          u"ST_Rotation"_s,
          u"ST_Roughness"_s,
          u"ST_SRID"_s,
          u"ST_SameAlignment"_s,
          u"ST_Scale"_s,
          u"ST_ScaleX"_s,
          u"ST_ScaleY"_s,
          u"ST_Segmentize"_s,
          u"ST_SetBandIndex"_s,
          u"ST_SetBandIsNoData"_s,
          u"ST_SetBandNoDataValue"_s,
          u"ST_SetBandPath"_s,
          u"ST_SetEffectiveArea"_s,
          u"ST_SetGeoReference"_s,
          u"ST_SetPoint"_s,
          u"ST_SetRotation"_s,
          u"ST_SetSRID"_s,
          u"ST_SetScale"_s,
          u"ST_SetSkew"_s,
          u"ST_SetUpperLeft"_s,
          u"ST_SetValue"_s,
          u"ST_SetValues"_s,
          u"ST_SharedPaths"_s,
          u"ST_ShiftLongitude"_s,
          u"ST_ShortestLine"_s,
          u"ST_Simplify"_s,
          u"ST_SimplifyPreserveTopology"_s,
          u"ST_SimplifyVW"_s,
          u"ST_SkewX"_s,
          u"ST_SkewY"_s,
          u"ST_Slope"_s,
          u"ST_Snap"_s,
          u"ST_SnapToGrid"_s,
          u"ST_Split"_s,
          u"ST_Square"_s,
          u"ST_SquareGrid"_s,
          u"ST_StartPoint"_s,
          u"ST_StdDev4ma"_s,
          u"ST_StraightSkeleton"_s,
          u"ST_Subdivide"_s,
          u"ST_Sum4ma"_s,
          u"ST_Summary"_s,
          u"ST_SummaryStats"_s,
          u"ST_SummaryStatsAgg"_s,
          u"ST_SwapOrdinates"_s,
          u"ST_SymDifference"_s,
          u"ST_TPI"_s,
          u"ST_TRI"_s,
          u"ST_Tesselate"_s, //#spellok
          u"ST_Tile"_s,
          u"ST_TileEnvelope"_s,
          u"ST_Touches"_s,
          u"ST_TransScale"_s,
          u"ST_Transform"_s,
          u"ST_Translate"_s,
          u"ST_UnaryUnion"_s,
          u"ST_Union"_s,
          u"ST_UpperLeftX"_s,
          u"ST_UpperLeftY"_s,
          u"ST_Value"_s,
          u"ST_ValueCount"_s,
          u"ST_Volume"_s,
          u"ST_VoronoiLines"_s,
          u"ST_VoronoiPolygons"_s,
          u"ST_Width"_s,
          u"ST_Within"_s,
          u"ST_WorldToRasterCoord"_s,
          u"ST_WorldToRasterCoordX"_s,
          u"ST_WorldToRasterCoordY"_s,
          u"ST_WrapX"_s,
          u"ST_X"_s,
          u"ST_XMax"_s,
          u"ST_XMin"_s,
          u"ST_Y"_s,
          u"ST_YMax"_s,
          u"ST_YMin"_s,
          u"ST_Z"_s,
          u"ST_ZMax"_s,
          u"ST_ZMin"_s,
          u"ST_Zmflag"_s,
          u"Set_Geocode_Setting"_s,
          u"TopoElementArray_Agg"_s,
          u"TopoGeo_AddLineString"_s,
          u"TopoGeo_AddPoint"_s,
          u"TopoGeo_AddPolygon"_s,
          u"TopoGeom_addElement"_s,
          u"TopoGeom_remElement"_s,
          u"TopologySummary"_s,
          u"Topology_Load_Tiger"_s,
          u"UpdateGeometrySRID"_s,
          u"UpdateRasterSRID"_s,
          u"ValidateTopology"_s,
          u"box2d"_s,
          u"clearTopoGeom"_s,
          u"geometry_dump"_s,
          u"parse_address"_s,
          u"postgis.backend"_s,
          u"postgis.enable_outdb_rasters"_s,
          u"postgis.gdal_datapath"_s,
          u"postgis.gdal_enabled_drivers"_s,
          u"postgis_sfcgal_version"_s,
          u"standardize_address"_s,
          u"toTopoGeom"_s,
          u"|=|"_s,
          u"~"_s,
          u"~(box2df,box2df)"_s,
          u"~(box2df,geometry)"_s,
          u"~(geometry,box2df)"_s,
          u"~="_s,
        }
      }
    }
  );
}

QgsFields QgsPostgresProviderConnection::fields( const QString &schema, const QString &tableName, QgsFeedback *feedback ) const
{
  // Try the base implementation first and fall back to a more complex approach for the
  // few PG-specific corner cases that do not work with the base implementation.
  try
  {
    return QgsAbstractDatabaseProviderConnection::fields( schema, tableName, feedback );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    // This table might expose multiple geometry columns (different geom type or SRID)
    // but we are only interested in fields here, so let's pick the first one.
    TableProperty tableInfo { table( schema, tableName, feedback ) };
    if ( feedback && feedback->isCanceled() )
      return QgsFields();

    try
    {
      QgsDataSourceUri tUri { tableUri( schema, tableName ) };

      if ( tableInfo.geometryColumnTypes().count() > 1 )
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

      tUri.setParam( u"checkPrimaryKeyUnicity"_s, "0"_L1 );
      QgsVectorLayer::LayerOptions options { false, true };
      options.skipCrsValidation = true;

      QgsVectorLayer vl { tUri.uri(), u"temp_layer"_s, mProviderKey, options };

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

void QgsPostgresProviderConnection::renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const
{
  executeSqlPrivate( u"ALTER TABLE %1.%2 RENAME COLUMN %3 TO %4;"_s
                       .arg( QgsPostgresConn::quotedIdentifier( schema ), QgsPostgresConn::quotedIdentifier( tableName ), QgsPostgresConn::quotedIdentifier( name ), QgsPostgresConn::quotedIdentifier( newName ) ) );
}

void QgsPostgresProviderConnection::moveTableToSchema( const QString &sourceSchema, const QString &tableName, const QString &targetSchema ) const
{
  const QString sqlMoveToSchema = u"ALTER TABLE %1.%2 SET SCHEMA %3;"_s;

  const QString sqlMoveTable = sqlMoveToSchema.arg( QgsPostgresConn::quotedIdentifier( sourceSchema ), QgsPostgresConn::quotedIdentifier( tableName ), QgsPostgresConn::quotedIdentifier( targetSchema ) );

  auto conn = std::make_shared<QgsPoolPostgresConn>( QgsPostgresConn::connectionInfo( QgsDataSourceUri( uri() ), false ) );
  QgsPostgresLayerProperty property;
  // need property from target schema, it is already moved
  bool ok = conn->get()->supportedLayer( property, sourceSchema, tableName );

  if ( !ok )
  {
    throw QgsProviderConnectionException( u"Table `%1` requested for move, does not exist."_s.arg( tableName ) );
  }

  QString sqlAdditionalCommands;
  // if raster table is moved the overview info is not updated so we need to do it manually
  // also the overviews are moved to the same schema as the raster
  if ( property.isRaster )
  {
    // first take a look if there were overviews for the moved raster
    const QString sqlOverviews = u"SELECT o_table_schema, o_table_name, o_raster_column, overview_factor FROM public.raster_overviews WHERE r_table_schema = %1 AND r_table_name = %2;"_s
                                   .arg( QgsPostgresConn::quotedValue( sourceSchema ), QgsPostgresConn::quotedValue( tableName ) );

    const QList<QVariantList> results = executeSqlPrivate( sqlOverviews );

    for ( const QVariantList &result : std::as_const( results ) )
    {
      const QString overviewSchema = result.at( 0 ).toString();
      const QString overviewTableName = result.at( 1 ).toString();
      const QString overviewRastCol = result.at( 2 ).toString();
      const QVariant overviewFactor = result.at( 3 );

      // drop the overview constraint
      const QString sqlDropConstraint = u"SELECT DropOverviewConstraints(%1, %2, %3);"_s
                                          .arg( QgsPostgresConn::quotedValue( overviewSchema ), QgsPostgresConn::quotedValue( overviewTableName ), QgsPostgresConn::quotedValue( overviewRastCol ) );
      sqlAdditionalCommands.append( sqlDropConstraint );

      // move overview table to the target schema
      const QString sqlMoveOverview = sqlMoveToSchema.arg( QgsPostgresConn::quotedIdentifier( overviewSchema ), QgsPostgresConn::quotedIdentifier( overviewTableName ), QgsPostgresConn::quotedIdentifier( targetSchema ) );
      sqlAdditionalCommands.append( sqlMoveOverview );

      // create the overview constraint with updated info
      const QString sqlAddConstraint = u"SELECT AddOverviewConstraints(%1, %2, %3, %4, %5, %6, %7);"_s
                                         .arg( QgsPostgresConn::quotedValue( targetSchema ), QgsPostgresConn::quotedValue( overviewTableName ), QgsPostgresConn::quotedValue( overviewRastCol ), QgsPostgresConn::quotedValue( targetSchema ), QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( property.geometryColName ), QgsPostgresConn::quotedValue( overviewFactor ) );
      sqlAdditionalCommands.append( sqlAddConstraint );
    }
  }

  conn->get()->begin();

  QgsPostgresResult resMove( conn->get()->LoggedPQexec( "QgsPostgresProviderConnection", u"%1 %2"_s.arg( sqlMoveTable ).arg( sqlAdditionalCommands ) ) );

  if ( resMove.PQresultStatus() == PGRES_FATAL_ERROR )
  {
    conn->get()->rollback();
    throw QgsProviderConnectionException( u"Cannot move `%1` to schema `%2`."_s.arg( tableName ).arg( targetSchema ) );
  }

  conn->get()->commit();
}
