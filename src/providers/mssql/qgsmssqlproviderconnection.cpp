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

#include "qgsmssqlproviderconnection.h"
#include "qgsmssqlconnection.h"
#include "qgssettings.h"
#include "qgsmssqlprovider.h"
#include "qgsexception.h"
#include "qgsapplication.h"


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
  // Remove the sql and table empty parts
  const QRegularExpression removePartsRe { R"raw(\s*sql=\s*|\s*table=""\s*)raw" };
  setUri( QgsMssqlConnection::connUri( name ).uri().replace( removePartsRe, QString() ) );
  setDefaultCapabilities();
}

QgsMssqlProviderConnection::QgsMssqlProviderConnection( const QString &uri, const QVariantMap &configuration ):
  QgsAbstractDatabaseProviderConnection( QString(), configuration )
{
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
    Capability::TableExists
  };
}

void QgsMssqlProviderConnection::dropTablePrivate( const QString &schema, const QString &name ) const
{
  const QString sql = QString( "DROP TABLE %1.%2\n"
                               "DELETE FROM geometry_columns WHERE f_table_schema = '%3' AND f_table_name = '%4'" )
                      .arg( QgsMssqlProvider::quotedIdentifier( schema ) )
                      .arg( QgsMssqlProvider::quotedIdentifier( name ) )
                      .arg( QgsMssqlProvider::quotedValue( schema ) )
                      .arg( QgsMssqlProvider::quotedValue( name ) );

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
  QgsVectorLayerExporter::ExportError errCode = QgsMssqlProvider::createEmptyLayer(
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


void QgsMssqlProviderConnection::createSchema( const QString &name ) const
{
  checkCapability( Capability::CreateSchema );
  executeSqlPrivate( QStringLiteral( "CREATE SCHEMA %1" )
                     .arg( QgsMssqlProvider::quotedIdentifier( name ) ) );

}

void QgsMssqlProviderConnection::dropSchema( const QString &name,  bool force ) const
{
  checkCapability( Capability::DropSchema );
  executeSqlPrivate( QStringLiteral( "DROP SCHEMA %1 %2" )
                     .arg( QgsMssqlProvider::quotedIdentifier( name ) )
                     .arg( force ? QStringLiteral( "CASCADE" ) : QString() ) );
}

QList<QVariantList> QgsMssqlProviderConnection::executeSql( const QString &sql ) const
{
  checkCapability( Capability::ExecuteSql );
  return executeSqlPrivate( sql );
}

QList<QVariantList> QgsMssqlProviderConnection::executeSqlPrivate( const QString &sql, bool resolveTypes ) const
{
  const QgsDataSourceUri dsUri { uri() };
  QList<QVariantList> results;
  // connect to database
  QSqlDatabase db = QgsMssqlConnection::getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !QgsMssqlConnection::openDatabase( db ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Connection to %1 failed: %2" )
                                          .arg( uri() )
                                          .arg( db.lastError().text() ) );
  }
  else
  {

    QSqlQuery q = QSqlQuery( db );
    q.setForwardOnly( true );

    if ( ! q.exec( sql ) )
    {
      const QString errorMessage { q.lastError().text() };
      throw QgsProviderConnectionException( QObject::tr( "SQL error: %1 \n %2" )
                                            .arg( sql )
                                            .arg( errorMessage ) );

    }

    if ( q.isActive() )
    {
      QSqlRecord rec { q.record() };
      const int numCols { rec.count() };
      while ( q.next() )
      {
        QVariantList row;
        for ( int col = 0; col < numCols; ++col )
        {
          row.push_back( q.value( col ).toString() );
        }
        results.push_back( row );
      }
    }

  }
  return results;
}

QList<QgsMssqlProviderConnection::TableProperty> QgsMssqlProviderConnection::tables( const QString &schema, const TableFlags &flags ) const
{
  checkCapability( Capability::Tables );
  QList<QgsMssqlProviderConnection::TableProperty> tables;
  QString errCause;
  // TODO: set flags from the connection if flags argument is 0
  const QgsDataSourceUri dsUri { uri() };

  const bool useGeometryColumns { dsUri.hasParam( QStringLiteral( "geometryColumnsOnly" ) )
                                  &&( dsUri.param( QStringLiteral( "geometryColumnsOnly" ) ) == QStringLiteral( "true" )
                                      || dsUri.param( QStringLiteral( "geometryColumnsOnly" ) ) == '1' ) };

  const bool allowGeometrylessTables { dsUri.hasParam( QStringLiteral( "allowGeometrylessTables" ) )
                                       &&( dsUri.param( QStringLiteral( "allowGeometrylessTables" ) ) == QStringLiteral( "true" )
                                           || dsUri.param( QStringLiteral( "allowGeometrylessTables" ) ) == '1' ) };

  QString query { QStringLiteral( "SELECT " ) };

  if ( useGeometryColumns )
  {
    query += QStringLiteral( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0 FROM geometry_columns WHERE f_table_schema = %1" )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
  }
  else
  {
    query += QStringLiteral( "sys.schemas.name, sys.objects.name, sys.columns.name, null, "
                             "'GEOMETRY', CASE WHEN sys.objects.type = 'V' THEN 1 ELSE 0 eEND "
                             "FROM sys.columns JOIN sys.types ON sys.columns.system_type_id = sys.types.system_type_id AND "
                             "sys.columns.user_type_id = sys.types.user_type_id "
                             "JOIN sys.objects ON sys.objects.object_id = sys.columns.object_id JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id "
                             "WHERE sys.schemas.name = %1 AND (sys.types.name = 'geometry' OR sys.types.name = 'geography') "
                             "AND (sys.objects.type = 'U' or sys.objects.type = 'V') " )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
  }

  if ( allowGeometrylessTables )
  {
    query += QStringLiteral( "UNION ALL SELECT sys.schemas.name, sys.objects.name, null, null, 'NONE', "
                             "CASE WHEN sys.objects.type = 'V' THEN 1 ELSE 0 END from sys.objects JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id WHERE NOT EXISTS"
                             "(SELECT * FROM sys.columns sc1 join sys.types on sc1.system_type_id = sys.types.system_type_id "
                             "WHERE sys.schemas.name = %1 AND  (sys.types.name = 'geometry' or sys.types.name = 'geography') "
                             "AND sys.objects.object_id = sc1.object_id) AND (sys.objects.type = 'U' or sys.objects.type = 'V')" )
             .arg( QgsMssqlProvider::quotedValue( schema ) );
  }

  const bool disableInvalidGeometryHandling { dsUri.hasParam( QStringLiteral( "disableInvalidGeometryHandling" ) )
      &&( dsUri.param( QStringLiteral( "disableInvalidGeometryHandling" ) ) == QStringLiteral( "true" )
          || dsUri.param( QStringLiteral( "disableInvalidGeometryHandling" ) ) == '1' ) };


  const QList<QVariantList> results { executeSqlPrivate( query, false ) };
  for ( const auto &row : results )
  {

    Q_ASSERT( row.count( ) == 6 );

    QgsMssqlProviderConnection::TableProperty table;
    table.setSchema( row[0].toString() );
    table.setTableName( row[1].toString() );
    table.setGeometryColumn( row[2].toString() );
    // [3] srid
    // [4] type
    if ( row[5].toBool() )
      table.setFlag( QgsMssqlProviderConnection::TableFlag::View );

    tables.push_back( table );
  }

  // Fill in geometry type
  /*

  query = QStringLiteral( "SELECT %3"
                           " UPPER([%1].STGeometryType()),"
                           " [%1].STSrid"
                           " FROM %2"
                           " WHERE [%1] IS NOT NULL %4"
                           " GROUP BY [%1].STGeometryType(), [%1].STSrid" )
                  .arg( layerProperty.geometryColName,
                        table,
                        mUseEstimatedMetadata ? "TOP 1" : "",
                        layerProperty.sql.isEmpty() ? QString() : QStringLiteral( " AND %1" ).arg( layerProperty.sql ) );
  */
  return tables;
}

QStringList QgsMssqlProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  QStringList schemas;
  QString errCause;
  const QgsDataSourceUri dsUri { uri() };
  const QString sql
  {
    "SELECT s.name AS schema_name, "
    "    s.schema_id, "
    "    u.name AS schema_owner "
    "FROM sys.schemas s "
    "    INNER JOIN sys.sysusers u "
    "        ON u.uid = s.principal_id "
    " WHERE u.issqluser = 1 "
    "    AND u.name NOT IN ('sys', 'guest', 'INFORMATION_SCHEMA')"
  };
  const QList<QVariantList> result { executeSqlPrivate( sql, false ) };
  for ( const auto &row : result )
  {
    if ( row.size() > 0 )
      schemas.push_back( row.at( 0 ).toString() );
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

