/***************************************************************************
  qgsmssqlproviderconnection.cpp - QgsMssqlProviderConnection

 ---------------------
 begin                : 10.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlproviderconnection.h"

#include <chrono>

#include "qgsapplication.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldatabase.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqlsqlquerybuilder.h"
#include "qgsmssqlutils.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QIcon>
#include <QSqlField>
#include <QSqlRecord>

const QStringList QgsMssqlProviderConnection::EXTRA_CONNECTION_PARAMETERS {
  u"geometryColumnsOnly"_s,
  u"allowGeometrylessTables"_s,
  u"disableInvalidGeometryHandling"_s,
  u"saveUsername"_s,
  u"savePassword"_s,
};

QgsMssqlProviderConnection::QgsMssqlProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = u"mssql"_s;
  // Remove the sql and table empty parts
  setUri( QgsMssqlConnection::connUri( name ).uri() );
  setDefaultCapabilities();
}

QgsMssqlProviderConnection::QgsMssqlProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( QString(), configuration )
{
  mProviderKey = u"mssql"_s;
  // Additional connection information
  const QgsDataSourceUri inputUri( uri );
  QgsDataSourceUri currentUri { QgsDataSourceUri( uri ).connectionInfo( false ) };

  if ( inputUri.hasParam( u"estimatedMetadata"_s ) )
  {
    currentUri.setUseEstimatedMetadata( inputUri.param( u"estimatedMetadata"_s ) == "true"_L1 || inputUri.param( u"estimatedMetadata"_s ) == '1' );
  }

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( inputUri.hasParam( param ) )
    {
      currentUri.setParam( param, inputUri.param( param ) );
    }
  }

  if ( inputUri.hasParam( u"excludedSchemas"_s ) )
    currentUri.setParam( u"excludedSchemas"_s, inputUri.param( u"excludedSchemas"_s ) );

  setUri( currentUri.uri() );
  setDefaultCapabilities();
}

void QgsMssqlProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities = {
    Capability::DropVectorTable,
    Capability::CreateVectorTable,
    Capability::RenameVectorTable,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::ExecuteSql,
    Capability::SqlLayers,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists,
    Capability::RenameField,
    Capability::DeleteField,
    Capability::DeleteFieldCascade,
    Capability::AddField,
    Capability::MoveTableToSchema
  };
  mGeometryColumnCapabilities = {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities = {
    Qgis::SqlLayerDefinitionCapability::SubsetStringFilter,
    Qgis::SqlLayerDefinitionCapability::PrimaryKeys,
    Qgis::SqlLayerDefinitionCapability::GeometryColumn,
    Qgis::SqlLayerDefinitionCapability::UnstableFeatureIds,
  };
}

void QgsMssqlProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  // Drop all constraints and delete the table
  const QString sql { QStringLiteral( R"raw(
  DECLARE @database nvarchar(50)
  DECLARE @table nvarchar(50)
  DECLARE @schema nvarchar(50)

  set @database = %1
  set @table = %2
  set @schema = %3

  DECLARE @sql nvarchar(255)
  WHILE EXISTS(select * from INFORMATION_SCHEMA.TABLE_CONSTRAINTS where CONSTRAINT_CATALOG = @database and TABLE_NAME = @table AND TABLE_SCHEMA = @schema )
  BEGIN
      select    @sql = 'ALTER TABLE [' + @schema + '].[' + @table + '] DROP CONSTRAINT [' + CONSTRAINT_NAME + ']'
      from    INFORMATION_SCHEMA.TABLE_CONSTRAINTS
      where    constraint_catalog = @database and
              table_name = @table and table_schema = @schema
      exec    sp_executesql @sql
  END

  DROP TABLE %5.%4

  if exists (select * from INFORMATION_SCHEMA.TABLES where TABLE_NAME = 'geometry_columns' )
     DELETE FROM geometry_columns WHERE f_table_schema = @schema AND f_table_name = @table

  )raw" )
                        .arg( QgsMssqlUtils::quotedValue( u"master"_s ), // in my testing docker, it is 'master' instead of QgsMssqlUtils::quotedValue( QgsDataSourceUri( uri() ).database() ),
                              QgsMssqlUtils::quotedValue( name ), QgsMssqlUtils::quotedValue( schema ), QgsMssqlUtils::quotedIdentifier( name ), QgsMssqlUtils::quotedIdentifier( schema ) ) };

  executeSqlPrivate( sql );
}

void QgsMssqlProviderConnection::renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const
{
  executeSqlPrivate( u"EXECUTE sp_rename '%1.%2', %3"_s
                       .arg( QgsMssqlUtils::quotedIdentifier( schema ), QgsMssqlUtils::quotedIdentifier( name ), QgsMssqlUtils::quotedValue( newName ) ) );
}

void QgsMssqlProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const
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
  const Qgis::VectorExportResult res = QgsMssqlProvider::createEmptyLayer(
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

QString QgsMssqlProviderConnection::createVectorLayerExporterDestinationUri( const VectorLayerExporterOptions &options, QVariantMap &providerOptions ) const
{
  QgsDataSourceUri destUri( uri() );

  destUri.setTable( options.layerName );
  destUri.setSchema( options.schema );
  destUri.setGeometryColumn( options.wkbType != Qgis::WkbType::NoGeometry ? ( options.geometryColumn.isEmpty() ? u"geom"_s : options.geometryColumn ) : QString() );
  if ( !options.primaryKeyColumns.isEmpty() )
  {
    if ( options.primaryKeyColumns.length() > 1 )
    {
      QgsMessageLog::logMessage( u"Multiple primary keys are not supported by SQL Server, ignoring"_s, QString(), Qgis::MessageLevel::Info );
    }
    destUri.setKeyColumn( options.primaryKeyColumns.at( 0 ) );
  }

  providerOptions.clear();

  return destUri.uri( false );
}

QString QgsMssqlProviderConnection::tableUri( const QString &schema, const QString &name ) const
{
  const auto tableInfo { table( schema, name ) };
  QgsDataSourceUri dsUri( uri() );
  dsUri.setTable( name );
  dsUri.setSchema( schema );
  return dsUri.uri( false );
}

void QgsMssqlProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  dropTablePrivate( schema, name );
}

void QgsMssqlProviderConnection::renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const
{
  checkCapability( Capability::RenameVectorTable );
  renameTablePrivate( schema, name, newName );
}

void QgsMssqlProviderConnection::createSchema( const QString &schemaName ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( u"CREATE SCHEMA %1"_s
                       .arg( QgsMssqlUtils::quotedIdentifier( schemaName ) ) );
}

void QgsMssqlProviderConnection::dropSchema( const QString &schemaName, bool force ) const
{
  checkCapability( Capability::DropSchema );
  // We need to delete all tables first!
  // Note: there might be more linked objects to drop but MSSQL sucks so let's stick to the
  //       easiest case.
  if ( force )
  {
    const auto schemaTables { tables( schemaName ) };
    for ( const auto &t : schemaTables )
    {
      dropTablePrivate( schemaName, t.tableName() );
    }
  }
  executeSqlPrivate( u"DROP SCHEMA %1"_s
                       .arg( QgsMssqlUtils::quotedIdentifier( schemaName ) ) );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsMssqlProviderConnection::execSql( const QString &sql, QgsFeedback *feedback ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql, true, feedback );
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsMssqlProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes, QgsFeedback *feedback ) const
{
  if ( feedback && feedback->isCanceled() )
  {
    return QgsAbstractDatabaseProviderConnection::QueryResult();
  }

  const QgsDataSourceUri dsUri { uri() };

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );

  if ( !db->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection to %1 failed: %2" )
                                            .arg( uri(), db->errorText() ) );
  }
  else
  {
    if ( feedback && feedback->isCanceled() )
    {
      return QgsAbstractDatabaseProviderConnection::QueryResult();
    }

    //qDebug() << "MSSQL QUERY:" << sql;

    auto q = std::make_unique<QgsMssqlQuery>( db );
    q->setForwardOnly( true );
    QgsDatabaseQueryLogWrapper logWrapper { sql, uri(), providerKey(), u"QgsMssqlProviderConnection"_s, QGS_QUERY_LOG_ORIGIN };


    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    if ( !q->exec( sql ) )
    {
      const QString errorMessage { q->lastError().text() };
      logWrapper.setError( errorMessage );
      throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 \n %2" )
                                              .arg( sql, errorMessage ) );
    }

    if ( q->isActive() )
    {
      const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      const QSqlRecord rec { q->record() };
      const int numCols { rec.count() };
      const auto iterator = std::make_shared<QgssMssqlProviderResultIterator>( resolveTypes, numCols, std::move( q ) );
      QgsAbstractDatabaseProviderConnection::QueryResult results( iterator );
      results.setQueryExecutionTime( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() );
      for ( int idx = 0; idx < numCols; ++idx )
      {
        results.appendColumn( rec.field( idx ).name() );
      }
      return results;
    }
  }
  return QgsAbstractDatabaseProviderConnection::QueryResult();
}


QgssMssqlProviderResultIterator::QgssMssqlProviderResultIterator( bool resolveTypes, int columnCount, std::unique_ptr<QgsMssqlQuery> query )
  : mResolveTypes( resolveTypes )
  , mColumnCount( columnCount )
  , mQuery( std::move( query ) )
{
  // Load first row
  nextRow();
}

QVariantList QgssMssqlProviderResultIterator::nextRowPrivate()
{
  const QVariantList currentRow = mNextRow;
  mNextRow = nextRowInternal();
  return currentRow;
}

bool QgssMssqlProviderResultIterator::hasNextRowPrivate() const
{
  return !mNextRow.isEmpty();
}

QVariantList QgssMssqlProviderResultIterator::nextRowInternal()
{
  QVariantList row;
  if ( mQuery->next() )
  {
    for ( int col = 0; col < mColumnCount; ++col )
    {
      if ( mResolveTypes )
      {
        row.push_back( mQuery->value( col ) );
      }
      else
      {
        row.push_back( mQuery->value( col ).toString() );
      }
    }
  }
  else
  {
    mQuery->finish();
  }
  return row;
}

long long QgssMssqlProviderResultIterator::rowCountPrivate() const
{
  return mQuery->size();
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsMssqlProviderConnection::table( const QString &schema, const QString &table, QgsFeedback *feedback ) const
{
  const QList<QgsMssqlProviderConnection::TableProperty> properties { tablesPrivate( schema, table, TableFlags(), feedback ) };
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

QList<QgsMssqlProviderConnection::TableProperty> QgsMssqlProviderConnection::tables( const QString &schema, const TableFlags &flags, QgsFeedback *feedback ) const
{
  return tablesPrivate( schema, QString(), flags, feedback );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsMssqlProviderConnection::tablesPrivate( const QString &schema, const QString &table, const TableFlags &flags, QgsFeedback *feedback ) const
{
  checkCapability( Capability::Tables );
  QList<QgsMssqlProviderConnection::TableProperty> tables;

  const QgsDataSourceUri dsUri { uri() };

  // Defaults to false
  const bool useGeometryColumnsOnly { !table.isEmpty() && ( dsUri.hasParam( u"geometryColumnsOnly"_s ) && ( dsUri.param( u"geometryColumnsOnly"_s ) == "true"_L1 || dsUri.param( u"geometryColumnsOnly"_s ) == '1' ) ) };

  // Defaults to true
  const bool useEstimatedMetadata { !dsUri.hasParam( u"estimatedMetadata"_s ) || ( dsUri.param( u"estimatedMetadata"_s ) == "true"_L1 || dsUri.param( u"estimatedMetadata"_s ) == '1' ) };

  // Defaults to true because we want to list all tables if flags are not set
  bool allowGeometrylessTables;
  if ( flags == 0 || !table.isEmpty() )
  {
    allowGeometrylessTables = true;
  }
  else
  {
    allowGeometrylessTables = flags.testFlag( TableFlag::Aspatial );
  }
  const bool disableInvalidGeometryHandling = dsUri.hasParam( u"disableInvalidGeometryHandling"_s ) && dsUri.param( u"disableInvalidGeometryHandling"_s ).toInt();
  QString tableNameFilter;
  if ( !table.isEmpty() )
  {
    tableNameFilter = u" AND sys.objects.name = %1"_s.arg( QgsMssqlUtils::quotedValue( table ) );
  }

  QString query { u"SELECT "_s };

  if ( useGeometryColumnsOnly )
  {
    query += u"f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0 FROM geometry_columns WHERE f_table_schema = %1"_s
               .arg( QgsMssqlUtils::quotedValue( schema ) );
  }
  else
  {
    query += QStringLiteral( R"raw(
                             sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY', CASE WHEN sys.objects.type = 'V' THEN 1 ELSE 0 END
                              FROM sys.columns
                                JOIN sys.types
                                  ON sys.columns.system_type_id = sys.types.system_type_id AND sys.columns.user_type_id = sys.types.user_type_id
                                JOIN sys.objects
                                  ON sys.objects.object_id = sys.columns.object_id
                                JOIN sys.schemas
                                  ON sys.objects.schema_id = sys.schemas.schema_id
                                WHERE
                                  sys.schemas.name = %1 %2
                                  AND (sys.types.name = 'geometry' OR sys.types.name = 'geography')
                                  AND (sys.objects.type = 'U' OR sys.objects.type = 'V')
                             )raw" )
               .arg( QgsMssqlUtils::quotedValue( schema ), tableNameFilter );
  }

  if ( allowGeometrylessTables )
  {
    query += QStringLiteral( R"raw(
                             UNION ALL SELECT sys.schemas.name, sys.objects.name, null, null, 'NONE',
                               CASE WHEN sys.objects.type = 'V' THEN 1 ELSE 0 END
                             FROM sys.objects
                               JOIN sys.schemas
                                  ON sys.objects.schema_id = sys.schemas.schema_id
                             WHERE
                               sys.schemas.name = %1 %2
                               AND NOT EXISTS
                                (SELECT *
                                  FROM sys.columns sc1 JOIN sys.types ON sc1.system_type_id = sys.types.system_type_id
                                  WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography')
                                    AND sys.objects.object_id = sc1.object_id )
                               AND (sys.objects.type = 'U' OR sys.objects.type = 'V')
                             )raw" )
               .arg( QgsMssqlUtils::quotedValue( schema ), tableNameFilter );
  }

  const QList<QVariantList> results { executeSqlPrivate( query, false ).rows() };
  for ( const auto &row : results )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    Q_ASSERT( row.count() == 6 );
    QgsMssqlProviderConnection::TableProperty table;
    table.setSchema( row[0].toString() );
    table.setTableName( row[1].toString() );
    table.setGeometryColumn( row[2].toString() );
    //const QVariant srid = row[3];
    //const QVariant type = row[4]; // GEOMETRY|GEOGRAPHY
    if ( row[5].toBool() )
      table.setFlag( QgsMssqlProviderConnection::TableFlag::View );

    int geomColCount { 0 };

    if ( !table.geometryColumn().isEmpty() )
    {
      // Fetch geom cols
      QString geomColSql;

      if ( disableInvalidGeometryHandling )
      {
        // this query will fail if the table contains invalid geometries
        geomColSql = QStringLiteral( R"raw(
SELECT %4 UPPER( %1.STGeometryType()), %1.STSrid,
                            %1.HasZ, %1.HasM
                        FROM %2.%3
                        WHERE %1 IS NOT NULL
                        GROUP BY %1.STGeometryType(), %1.STSrid, %1.HasZ, %1.HasM

                        )raw" )
                       .arg( QgsMssqlUtils::quotedIdentifier( table.geometryColumn() ), QgsMssqlUtils::quotedIdentifier( table.schema() ), QgsMssqlUtils::quotedIdentifier( table.tableName() ), useEstimatedMetadata ? "TOP 1" : "" );
      }
      else
      {
        geomColSql = QStringLiteral( R"raw(
                        SELECT type, srid, hasz, hasm FROM
                            (SELECT %4 UPPER((CASE WHEN %1.STIsValid() = 0 THEN %1.MakeValid() ELSE %1 END).STGeometryType()) as type,
                             %1.STSrid as srid, %1.HasZ as hasz, %1.HasM as hasm FROM %2.%3 WHERE %1 IS NOT NULL) AS a
                        GROUP BY type, srid, hasz, hasm
                        )raw" )
                       .arg( QgsMssqlUtils::quotedIdentifier( table.geometryColumn() ), QgsMssqlUtils::quotedIdentifier( table.schema() ), QgsMssqlUtils::quotedIdentifier( table.tableName() ), useEstimatedMetadata ? "TOP 1" : "" );
      }

      try
      {
        const auto geomColResults { executeSqlPrivate( geomColSql ).rows() };
        for ( const auto &row : geomColResults )
        {
          Qgis::WkbType geometryType { QgsWkbTypes::parseType( row[0].toString() ) };
          if ( row[2].toString() == '1' )
          {
            geometryType = QgsWkbTypes::addZ( geometryType );
          }
          if ( row[3].toString() == '1' )
          {
            geometryType = QgsWkbTypes::addM( geometryType );
          }
          table.addGeometryColumnType( geometryType, QgsCoordinateReferenceSystem::fromEpsgId( row[1].toLongLong() ) );
          ++geomColCount;
        }
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error retrieving geometry type for '%1' on table %2.%3:\n%4" ).arg( table.geometryColumn(), QgsMssqlUtils::quotedIdentifier( table.schema() ), QgsMssqlUtils::quotedIdentifier( table.tableName() ), ex.what() ), u"MSSQL"_s, Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      // Add an invalid column
      table.addGeometryColumnType( Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() );
      table.setFlag( QgsMssqlProviderConnection::TableFlag::Aspatial );
    }

    table.setGeometryColumnCount( geomColCount );
    tables.push_back( table );
  }
  return tables;
}

QgsFields QgsMssqlProviderConnection::fields( const QString &schema, const QString &table, QgsFeedback *feedback ) const
{
  if ( feedback && feedback->isCanceled() )
  {
    return QgsFields();
  }

  const QgsDataSourceUri dsUri { uri() };

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );
  if ( !db->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection to %1 failed: %2" )
                                            .arg( uri(), db->errorText() ) );
  }

  QgsMssqlDatabase::FieldDetails details;

  QString error;
  const bool result = db->loadFields( details, schema, table, error );
  if ( !result )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving fields information: %1" ).arg( error ) );
  }

  return details.attributeFields;
}

void QgsMssqlProviderConnection::renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const
{
  executeSqlPrivate( u"EXECUTE sp_rename '%1.%2.%3', %4, 'COLUMN'"_s
                       .arg( QgsMssqlUtils::quotedIdentifier( schema ), QgsMssqlUtils::quotedIdentifier( tableName ), QgsMssqlUtils::quotedIdentifier( name ), QgsMssqlUtils::quotedValue( newName ) ) );
}

QStringList QgsMssqlProviderConnection::schemas() const
{
  checkCapability( Capability::Schemas );

  QString errorMessage;
  const QStringList allSchemas = QgsMssqlConnection::schemas( uri(), &errorMessage );
  if ( !errorMessage.isEmpty() )
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving schemas: %1" ).arg( errorMessage ) );

  const QgsDataSourceUri connUri { uri() };

  QStringList excludedSchemaList;
  if ( connUri.hasParam( u"excludedSchemas"_s ) )
    excludedSchemaList = QgsDataSourceUri( uri() ).param( u"excludedSchemas"_s ).split( ',' );
  QStringList schemas;
  schemas.reserve( allSchemas.size() );
  for ( const QString &schema : allSchemas )
  {
    if ( QgsMssqlConnection::isSystemSchema( schema ) )
      continue;

    if ( excludedSchemaList.contains( schema ) )
      continue;

    schemas.push_back( schema );
  }
  return schemas;
}

void QgsMssqlProviderConnection::store( const QString &name ) const
{
  // TODO: move this to class configuration?
  const QString baseKey = u"/MSSQL/connections/"_s;
  // delete the original entry first
  remove( name );

  QgsSettings settings;
  settings.beginGroup( baseKey );
  settings.beginGroup( name );

  // From URI
  const QgsDataSourceUri dsUri { uri() };
  settings.setValue( "service", dsUri.service() );
  settings.setValue( "host", dsUri.host() );
  settings.setValue( "database", dsUri.database() );
  settings.setValue( "username", dsUri.username() );
  settings.setValue( "password", dsUri.password() );
  settings.setValue( "estimatedMetadata", dsUri.useEstimatedMetadata() );

  QgsMssqlConnection::setExcludedSchemasList( name, dsUri.database(), dsUri.param( u"excludedSchemas"_s ).split( ',' ) );

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( dsUri.hasParam( param ) )
    {
      settings.setValue( param, dsUri.param( param ) == "true"_L1 || dsUri.param( param ) == '1' );
    }
    else
    {
      settings.remove( param );
    }
  }

  // From configuration
  const auto config { configuration().keys() };
  for ( const auto &p : config )
  {
    if ( configuration().contains( p ) )
    {
      settings.setValue( p, configuration().value( p ) == "true"_L1 || configuration().value( p ) == '1' );
    }
    else
    {
      settings.remove( p );
    }
  }

  settings.endGroup();
  settings.endGroup();
}

void QgsMssqlProviderConnection::remove( const QString &name ) const
{
  const QString baseKey = u"/MSSQL/connections/"_s;
  QgsSettings settings;
  settings.remove( baseKey + name );
}

QIcon QgsMssqlProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconMssql.svg"_s );
}


QList<QgsVectorDataProvider::NativeType> QgsMssqlProviderConnection::nativeTypes() const
{
  return QgsMssqlConnection::nativeTypes();
}

QgsProviderSqlQueryBuilder *QgsMssqlProviderConnection::queryBuilder() const
{
  return new QgsMsSqlSqlQueryBuilder();
}

QgsVectorLayer *QgsMssqlProviderConnection::createSqlVectorLayer( const SqlVectorLayerOptions &options ) const
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
    tUri.setTable( u"(SELECT row_number() OVER (ORDER BY (SELECT NULL)) AS _uid%1_, * FROM (%2\n) AS _subq_%3_\n)"_s.arg( QString::number( pkId ), sanitizeSqlForQueryLayer( options.sql ), QString::number( sqlId ) ) );
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    tUri.setGeometryColumn( options.geometryColumn );

    const QString sql = QStringLiteral( "SELECT %3"
                                        " UPPER(%1.STGeometryType()),"
                                        " %1.STSrid,"
                                        " %1.HasZ,"
                                        " %1.HasM"
                                        " FROM (%2) AS _subq_"
                                        " WHERE %1 IS NOT NULL %4"
                                        " GROUP BY %1.STGeometryType(), %1.STSrid, %1.HasZ, %1.HasM" )
                          .arg( QgsMssqlUtils::quotedIdentifier( options.geometryColumn ), sanitizeSqlForQueryLayer( options.sql ), tUri.useEstimatedMetadata() ? "TOP 1" : "", options.filter.isEmpty() ? QString() : u" AND %1"_s.arg( options.filter ) );

    try
    {
      const QList<QList<QVariant>> candidates { executeSql( sql ) };

      QStringList types;
      QStringList srids;

      for ( const QList<QVariant> &row : std::as_const( candidates ) )
      {
        const bool hasZ { row[2].toString() == '1' };
        const bool hasM { row[3].toString() == '1' };
        const int dimensions { 2 + ( ( hasZ && hasM ) ? 2 : ( ( hasZ || hasM ) ? 1 : 0 ) ) };
        QString typeName { row[0].toString().toUpper() };
        if ( typeName.isEmpty() )
          continue;

        if ( hasM && !typeName.endsWith( 'M' ) )
        {
          typeName.append( 'M' );
        }
        const QString type { QgsMssqlProvider::typeFromMetadata( typeName, dimensions ) };
        const QString srid = row[1].toString();

        if ( type.isEmpty() )
          continue;

        types << type;
        srids << srid;
      }

      if ( !srids.isEmpty() )
        tUri.setSrid( srids.at( 0 ) );

      if ( !types.isEmpty() )
      {
        tUri.setWkbType( QgsMssqlUtils::wkbTypeFromGeometryType( types.at( 0 ) ) );
      }
    }
    catch ( QgsProviderConnectionException &e )
    {
      QgsDebugError( e.what() );
    }
  }

  QgsVectorLayer::LayerOptions vectorLayerOptions { false, true };
  vectorLayerOptions.skipCrsValidation = true;
  return new QgsVectorLayer { tUri.uri( false ), options.layerName.isEmpty() ? u"QueryLayer"_s : options.layerName, providerKey(), vectorLayerOptions };
}

bool QgsMssqlProviderConnection::validateSqlVectorLayer( const SqlVectorLayerOptions &options, QString &message ) const
{
  const QgsDataSourceUri dsUri { uri() };
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );
  if ( !db->isValid() )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection to %1 failed: %2" )
                                            .arg( uri(), db->errorText() ) );
  }

  QgsMssqlDatabase::FieldDetails details;

  // check that all query fields have explicit names -- if this is not done, we cannot run aggregates on the query such
  // as count(*)
  QString error;
  const bool result = db->loadQueryFields( details, sanitizeSqlForQueryLayer( options.sql ), error );
  if ( !result )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving fields information: %1" ).arg( error ) );
  }

  QStringList emptyFieldIndexes;
  int i = 1;
  for ( const QgsField &f : details.attributeFields )
  {
    if ( f.name().isEmpty() || f.name().startsWith( "__unnamed__"_L1 ) )
    {
      emptyFieldIndexes << QString::number( i );
    }
    i++;
  }

  if ( !emptyFieldIndexes.empty() )
  {
    if ( emptyFieldIndexes.length() == 1 )
    {
      message = QObject::tr( "Column %1 is unnamed. SQL Server requires that all columns computed in a query have an explicit name set. Please add an \"AS column_name\" argument for this column." ).arg( emptyFieldIndexes.at( 0 ) );
    }
    else
    {
      message = QObject::tr( "Columns %1 are unnamed. SQL Server requires that all columns computed in a query have an explicit name set. Please add an \"AS column_name\" argument for these columns." ).arg( emptyFieldIndexes.join( QObject::tr( ", " ) ) );
    }
    return false;
  }

  if ( !options.geometryColumn.isEmpty() )
  {
    // if trying to load as geometry, make sure we can determine geometry type
    const QString sql = QStringLiteral( "SELECT TOP 1"
                                        " UPPER(%1.STGeometryType()),"
                                        " %1.STSrid,"
                                        " %1.HasZ,"
                                        " %1.HasM"
                                        " FROM (%2) AS _subq_"
                                        " WHERE %1 IS NOT NULL %3"
                                        " GROUP BY %1.STGeometryType(), %1.STSrid, %1.HasZ, %1.HasM" )
                          .arg( QgsMssqlUtils::quotedIdentifier( options.geometryColumn ), sanitizeSqlForQueryLayer( options.sql ), options.filter.isEmpty() ? QString() : u" AND %1"_s.arg( options.filter ) );

    try
    {
      ( void ) executeSql( sql );
    }
    catch ( QgsProviderConnectionException &e )
    {
      message = e.what();
      return false;
    }
  }


  return true;
}

Qgis::DatabaseProviderTableImportCapabilities QgsMssqlProviderConnection::tableImportCapabilities() const
{
  return Qgis::DatabaseProviderTableImportCapability::SetGeometryColumnName | Qgis::DatabaseProviderTableImportCapability::SetPrimaryKeyName;
}

QString QgsMssqlProviderConnection::defaultPrimaryKeyColumnName() const
{
  return u"qgs_fid"_s;
}

void QgsMssqlProviderConnection::moveTableToSchema( const QString &sourceSchema, const QString &tableName, const QString &targetSchema ) const
{
  executeSqlPrivate( u"ALTER SCHEMA %1 TRANSFER %2.%3"_s
                       .arg( QgsMssqlUtils::quotedIdentifier( targetSchema ), QgsMssqlUtils::quotedIdentifier( sourceSchema ), QgsMssqlUtils::quotedIdentifier( tableName ) ) );
}

QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions QgsMssqlProviderConnection::sqlOptions( const QString &layerSource )
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
