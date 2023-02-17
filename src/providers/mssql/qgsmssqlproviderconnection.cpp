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

#include <QSqlRecord>
#include <QSqlField>

#include "qgsmssqlproviderconnection.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldatabase.h"
#include "qgssettings.h"
#include "qgsmssqlprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsfeedback.h"
#include "qgsmssqlsqlquerybuilder.h"
#include "qgsdbquerylog.h"
#include <QIcon>

#include <chrono>

const QStringList QgsMssqlProviderConnection::EXTRA_CONNECTION_PARAMETERS
{
  QStringLiteral( "geometryColumnsOnly" ),
  QStringLiteral( "allowGeometrylessTables" ),
  QStringLiteral( "disableInvalidGeometryHandling" ),
  QStringLiteral( "saveUsername" ),
  QStringLiteral( "savePassword" ),
};

QgsMssqlProviderConnection::QgsMssqlProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "mssql" );
  // Remove the sql and table empty parts
  setUri( QgsMssqlConnection::connUri( name ).uri() );
  setDefaultCapabilities();
}

QgsMssqlProviderConnection::QgsMssqlProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QString(), configuration )
{
  mProviderKey = QStringLiteral( "mssql" );
  // Additional connection information
  const QgsDataSourceUri inputUri( uri );
  QgsDataSourceUri currentUri { QgsDataSourceUri( uri ).connectionInfo( false ) };

  if ( inputUri.hasParam( QStringLiteral( "estimatedMetadata" ) ) )
  {
    currentUri.setUseEstimatedMetadata( inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == QStringLiteral( "true" )
                                        || inputUri.param( QStringLiteral( "estimatedMetadata" ) ) == '1' );
  }

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( inputUri.hasParam( param ) )
    {
      currentUri.setParam( param, inputUri.param( param ) );
    }
  }

  if ( inputUri.hasParam( QStringLiteral( "excludedSchemas" ) ) )
    currentUri.setParam( QStringLiteral( "excludedSchemas" ), inputUri.param( QStringLiteral( "excludedSchemas" ) ) );

  setUri( currentUri.uri() );
  setDefaultCapabilities();
}

void QgsMssqlProviderConnection::setDefaultCapabilities()
{
  // TODO: we might check at this point if the user actually has the privileges and return
  //       properly filtered capabilities instead of all of them
  mCapabilities =
  {
    Capability::DropVectorTable,
    Capability::CreateVectorTable,
    Capability::DropSchema,
    Capability::CreateSchema,
    Capability::ExecuteSql,
    Capability::Tables,
    Capability::Schemas,
    Capability::Spatial,
    Capability::TableExists,
    Capability::DeleteField,
    Capability::DeleteFieldCascade,
    Capability::AddField
  };
  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z,
    GeometryColumnCapability::M,
    GeometryColumnCapability::Curves
  };
  mSqlLayerDefinitionCapabilities =
  {
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

  set @database = N%1
  set @table = N%2
  set @schema = N%3

  DECLARE @sql nvarchar(255)
  WHILE EXISTS(select * from INFORMATION_SCHEMA.TABLE_CONSTRAINTS where CONSTRAINT_CATALOG = @database and TABLE_NAME = @table AND TABLE_SCHEMA = @schema )
  BEGIN
      select    @sql = 'ALTER TABLE ' + @table + ' DROP CONSTRAINT ' + CONSTRAINT_NAME
      from    INFORMATION_SCHEMA.TABLE_CONSTRAINTS
      where    constraint_catalog = @database and
              table_name = @table and table_schema = @schema
      exec    sp_executesql @sql
  END

  DROP TABLE %5.%4

  if exists (select * from INFORMATION_SCHEMA.TABLES where TABLE_NAME = 'geometry_columns' )
     DELETE FROM geometry_columns WHERE f_table_schema = @schema AND f_table_name = @table

  )raw" )
                      .arg( QgsMssqlProvider::quotedValue( QStringLiteral( "master" ) ),  // in my testing docker, it is 'master' instead of QgsMssqlProvider::quotedValue( QgsDataSourceUri( uri() ).database() ),
                            QgsMssqlProvider::quotedValue( name ),
                            QgsMssqlProvider::quotedValue( schema ),
                            QgsMssqlProvider::quotedIdentifier( name ),
                            QgsMssqlProvider::quotedIdentifier( schema ) ) };

  executeSqlPrivate( sql );
}

void QgsMssqlProviderConnection::createVectorTable( const QString &schema,
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
  const Qgis::VectorExportResult res = QgsMssqlProvider::createEmptyLayer(
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


void QgsMssqlProviderConnection::createSchema( const QString &schemaName ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" )
                     .arg( QgsMssqlProvider::quotedIdentifier( schemaName ) ) );

}

void QgsMssqlProviderConnection::dropSchema( const QString &schemaName,  bool force ) const
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
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1" )
                     .arg( QgsMssqlProvider::quotedIdentifier( schemaName ) ) );
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
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

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

    std::unique_ptr<QgsMssqlQuery> q = std::make_unique<QgsMssqlQuery>( db );
    q->setForwardOnly( true );
    QgsDatabaseQueryLogWrapper logWrapper { sql, uri(), providerKey(), QStringLiteral( "QgsMssqlProviderConnection" ), QGS_QUERY_LOG_ORIGIN };


    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    if ( ! q->exec( sql ) )
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


QgssMssqlProviderResultIterator::QgssMssqlProviderResultIterator( bool resolveTypes, int columnCount,  std::unique_ptr<QSqlQuery> query )
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
  return ! mNextRow.isEmpty();
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


QList<QgsMssqlProviderConnection::TableProperty> QgsMssqlProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsMssqlProviderConnection::TableProperty> tables;

  const QgsDataSourceUri dsUri { uri() };

  // Defaults to false
  const bool useGeometryColumnsOnly { dsUri.hasParam( QStringLiteral( "geometryColumnsOnly" ) )
                                      &&( dsUri.param( QStringLiteral( "geometryColumnsOnly" ) ) == QStringLiteral( "true" )
                                          || dsUri.param( QStringLiteral( "geometryColumnsOnly" ) ) == '1' ) };

  // Defaults to true
  const bool useEstimatedMetadata { ! dsUri.hasParam( QStringLiteral( "estimatedMetadata" ) )
                                    || ( dsUri.param( QStringLiteral( "estimatedMetadata" ) ) == QStringLiteral( "true" )
                                         || dsUri.param( QStringLiteral( "estimatedMetadata" ) ) == '1' ) };

  // Defaults to true because we want to list all tables if flags are not set
  bool allowGeometrylessTables;
  if ( flags == 0 )
  {
    allowGeometrylessTables = true;
  }
  else
  {
    allowGeometrylessTables = flags.testFlag( TableFlag::Aspatial );
  }

  QString query { QStringLiteral( "SELECT " ) };

  if ( useGeometryColumnsOnly )
  {
    query += QStringLiteral( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0 FROM geometry_columns WHERE f_table_schema = N%1" )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
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
                                  sys.schemas.name = N%1
                                  AND (sys.types.name = 'geometry' OR sys.types.name = 'geography')
                                  AND (sys.objects.type = 'U' OR sys.objects.type = 'V')
                             )raw" )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
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
                               sys.schemas.name = N%1
                               AND NOT EXISTS
                                (SELECT *
                                  FROM sys.columns sc1 JOIN sys.types ON sc1.system_type_id = sys.types.system_type_id
                                  WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography')
                                    AND sys.objects.object_id = sc1.object_id )
                               AND (sys.objects.type = 'U' OR sys.objects.type = 'V')
                             )raw" )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
  }

  const QList<QVariantList> results { executeSqlPrivate( query, false ).rows() };
  for ( const auto &row : results )
  {
    Q_ASSERT( row.count( ) == 6 );
    QgsMssqlProviderConnection::TableProperty table;
    table.setSchema( row[0].toString() );
    table.setTableName( row[1].toString() );
    table.setGeometryColumn( row[2].toString() );
    //const QVariant srid = row[3];
    //const QVariant type = row[4]; // GEOMETRY|GEOGRAPHY
    if ( row[5].toBool() )
      table.setFlag( QgsMssqlProviderConnection::TableFlag::View );

    int geomColCount { 0 };

    if ( ! table.geometryColumn().isEmpty() )
    {
      // Fetch geom cols
      const QString geomColSql
      {
        QStringLiteral( R"raw(
                        SELECT %4 UPPER( %1.STGeometryType()), %1.STSrid
                        FROM %2.%3
                        WHERE %1 IS NOT NULL
                        GROUP BY %1.STGeometryType(), %1.STSrid
                        )raw" )
        .arg( QgsMssqlProvider::quotedIdentifier( table.geometryColumn() ),
              QgsMssqlProvider::quotedIdentifier( table.schema() ),
              QgsMssqlProvider::quotedIdentifier( table.tableName() ),
              useEstimatedMetadata ? "TOP 1" : "" ) };

      // This may fail for invalid geometries
      try
      {
        const auto geomColResults { executeSqlPrivate( geomColSql ).rows() };
        for ( const auto &row : geomColResults )
        {
          table.addGeometryColumnType( QgsWkbTypes::parseType( row[0].toString() ),
                                       QgsCoordinateReferenceSystem::fromEpsgId( row[1].toLongLong( ) ) );
          ++geomColCount;
        }
      }
      catch ( QgsProviderConnectionException &ex )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error retrieving geometry type for '%1' on table %2.%3:\n%4" )
                                   .arg( table.geometryColumn(),
                                         QgsMssqlProvider::quotedIdentifier( table.schema() ),
                                         QgsMssqlProvider::quotedIdentifier( table.tableName() ),
                                         ex.what() ),
                                   QStringLiteral( "MSSQL" ), Qgis::MessageLevel::Warning );
      }

    }
    else
    {
      // Add an invalid column
      table.addGeometryColumnType( QgsWkbTypes::Type::NoGeometry,
                                   QgsCoordinateReferenceSystem() );
      table.setFlag( QgsMssqlProviderConnection::TableFlag::Aspatial );
    }

    table.setGeometryColumnCount( geomColCount );
    tables.push_back( table );
  }
  return tables;
}

QStringList QgsMssqlProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;

  const QgsDataSourceUri connUri( uri() );

  const QgsDataSourceUri dsUri { uri() };
  const QString sql
  {
    QStringLiteral(
      R"raw(
    SELECT s.name AS schema_name,
        s.schema_id,
        u.name AS schema_owner
    FROM sys.schemas s
        INNER JOIN sys.sysusers u
            ON u.uid = s.principal_id
     WHERE u.issqluser = 1
        AND u.name NOT IN ('sys', 'guest', 'INFORMATION_SCHEMA')
    )raw" )
  };

  const QList<QVariantList> result { executeSqlPrivate( sql, false ).rows() };

  QStringList excludedSchemaList;
  if ( connUri.hasParam( QStringLiteral( "excludedSchemas" ) ) )
    excludedSchemaList = QgsDataSourceUri( uri() ).param( QStringLiteral( "excludedSchemas" ) ).split( ',' );
  for ( const auto &row : result )
  {
    if ( row.size() > 0 )
    {
      const QString schema = row.at( 0 ).toString();
      if ( !excludedSchemaList.contains( schema ) )
        schemas.push_back( schema );
    }
  }
  return schemas;
}


void QgsMssqlProviderConnection::store( const QString &name ) const
{
  // TODO: move this to class configuration?
  const QString baseKey = QStringLiteral( "/MSSQL/connections/" );
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

  QgsMssqlConnection::setExcludedSchemasList( name, dsUri.database(), dsUri.param( QStringLiteral( "excludedSchemas" ) ).split( ',' ) );

  for ( const auto &param : EXTRA_CONNECTION_PARAMETERS )
  {
    if ( dsUri.hasParam( param ) )
    {
      settings.setValue( param, dsUri.param( param ) == QStringLiteral( "true" )
                         || dsUri.param( param ) == '1' );
    }
  }

  // From configuration
  const auto config { configuration().keys() };
  for ( const auto &p : config )
  {
    settings.setValue( p, configuration().value( p ) == QStringLiteral( "true" )
                       || configuration().value( p ) == '1' );
  }

  settings.endGroup();
  settings.endGroup();
}

void QgsMssqlProviderConnection::remove( const QString &name ) const
{
  const QString baseKey = QStringLiteral( "/MSSQL/connections/" );
  QgsSettings settings;
  settings.remove( baseKey + name );
}

QIcon QgsMssqlProviderConnection::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconMssql.svg" ) );
}


QList<QgsVectorDataProvider::NativeType> QgsMssqlProviderConnection::nativeTypes() const
{
  return QgsMssqlConnection::nativeTypes();
}

QgsProviderSqlQueryBuilder *QgsMssqlProviderConnection::queryBuilder() const
{
  return new QgsMsSqlSqlQueryBuilder();
}
