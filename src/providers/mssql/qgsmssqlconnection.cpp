/***************************************************************************
                             qgsmssqlconnection.cpp
                             ----------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlconnection.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqldatabase.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsdatasourceuri.h"
#include "qgsvariantutils.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QFile>


bool QgsMssqlConnection::geometryColumnsOnly( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/geometryColumnsOnly" ), false ).toBool();
}

void QgsMssqlConnection::setGeometryColumnsOnly( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/geometryColumnsOnly" ), enabled );
}

bool QgsMssqlConnection::extentInGeometryColumns( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/extentInGeometryColumns" ), false ).toBool();
}

void QgsMssqlConnection::setExtentInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/extentInGeometryColumns" ), enabled );
}

bool QgsMssqlConnection::primaryKeyInGeometryColumns( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/primaryKeyInGeometryColumns" ), false ).toBool();
}

void QgsMssqlConnection::setPrimaryKeyInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/primaryKeyInGeometryColumns" ), enabled );
}

bool QgsMssqlConnection::allowGeometrylessTables( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/allowGeometrylessTables" ), false ).toBool();
}

void QgsMssqlConnection::setAllowGeometrylessTables( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/allowGeometrylessTables" ), enabled );
}

bool QgsMssqlConnection::useEstimatedMetadata( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/estimatedMetadata" ), false ).toBool();
}

void QgsMssqlConnection::setUseEstimatedMetadata( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/estimatedMetadata" ), enabled );
}

bool QgsMssqlConnection::isInvalidGeometryHandlingDisabled( const QString &name )
{
  const QgsSettings settings;
  return settings.value( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/disableInvalidGeometryHandling" ), false ).toBool();
}

void QgsMssqlConnection::setInvalidGeometryHandlingDisabled( const QString &name, bool disabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + name + QStringLiteral( "/disableInvalidGeometryHandling" ), disabled );
}

bool QgsMssqlConnection::dropView( const QString &uri, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return false;
  }

  QSqlQuery q = QSqlQuery( db->db() );
  if ( !q.exec( QString( "DROP VIEW [%1].[%2]" ).arg( schema, table ) ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }

  return true;
}

bool QgsMssqlConnection::dropTable( const QString &uri, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return false;
  }

  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  const QString sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                               "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'" )
                        .arg( schema, table );
  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }

  return true;
}

bool QgsMssqlConnection::truncateTable( const QString &uri, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return false;
  }

  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  const QString sql = QStringLiteral( "TRUNCATE TABLE [%1].[%2]" ).arg( schema, table );
  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }

  return true;
}

bool QgsMssqlConnection::createSchema( const QString &uri, const QString &schemaName, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return false;
  }

  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  const QString sql = QStringLiteral( "CREATE SCHEMA [%1]" ).arg( schemaName );
  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return false;
  }

  return true;
}

QStringList QgsMssqlConnection::schemas( const QString &uri, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  return schemas( db, errorMessage );
}

QStringList QgsMssqlConnection::schemas( std::shared_ptr<QgsMssqlDatabase> db, QString *errorMessage )
{
  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return QStringList();
  }

  const QString sql = QStringLiteral( "select s.name as schema_name from sys.schemas s" );

  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  if ( !q.exec( sql ) )
  {
    if ( errorMessage )
      *errorMessage = q.lastError().text();
    return QStringList();
  }

  QStringList result;

  while ( q.next() )
  {
    const QString schemaName = q.value( 0 ).toString();
    result << schemaName;
  }
  return result;
}

bool QgsMssqlConnection::isSystemSchema( const QString &schema )
{
  static const QSet<QString> sSystemSchemas {
    QStringLiteral( "db_owner" ),
    QStringLiteral( "db_securityadmin" ),
    QStringLiteral( "db_accessadmin" ),
    QStringLiteral( "db_backupoperator" ),
    QStringLiteral( "db_ddladmin" ),
    QStringLiteral( "db_datawriter" ),
    QStringLiteral( "db_datareader" ),
    QStringLiteral( "db_denydatawriter" ),
    QStringLiteral( "db_denydatareader" ),
    QStringLiteral( "INFORMATION_SCHEMA" ),
    QStringLiteral( "sys" )
  };

  return sSystemSchemas.contains( schema );
}

QgsDataSourceUri QgsMssqlConnection::connUri( const QString &connName )
{
  const QgsSettings settings;

  const QString key = QStringLiteral( "/MSSQL/connections/" ) + connName;

  const QString service = settings.value( key + QStringLiteral( "/service" ) ).toString();
  const QString host = settings.value( key + QStringLiteral( "/host" ) ).toString();
  const QString database = settings.value( key + QStringLiteral( "/database" ) ).toString();
  const QString username = settings.value( key + QStringLiteral( "/username" ) ).toString();
  const QString password = settings.value( key + QStringLiteral( "/password" ) ).toString();

  const bool useGeometryColumns { QgsMssqlConnection::geometryColumnsOnly( connName ) };
  const bool useEstimatedMetadata { QgsMssqlConnection::useEstimatedMetadata( connName ) };
  const bool allowGeometrylessTables { QgsMssqlConnection::allowGeometrylessTables( connName ) };
  const bool disableGeometryHandling { QgsMssqlConnection::isInvalidGeometryHandlingDisabled( connName ) };

  QgsDataSourceUri uri;
  if ( !service.isEmpty() )
  {
    uri.setConnection( service, database, username, password );
  }
  else
  {
    uri.setConnection( host, QString(), database, username, password );
  }

  uri.setParam( QStringLiteral( "geometryColumnsOnly" ), useGeometryColumns ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setParam( QStringLiteral( "allowGeometrylessTables" ), allowGeometrylessTables ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  uri.setParam( QStringLiteral( "disableInvalidGeometryHandling" ), disableGeometryHandling ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );

  if ( settings.value( QStringLiteral( "saveUsername" ) ).isValid() )
  {
    const bool saveUsername { settings.value( QStringLiteral( "saveUsername" ) ).toBool() };
    uri.setParam( QStringLiteral( "saveUsername" ), saveUsername ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    if ( !saveUsername )
    {
      uri.setUsername( QString() );
    }
  }
  if ( settings.value( QStringLiteral( "savePassword" ) ).isValid() )
  {
    const bool savePassword { settings.value( QStringLiteral( "savePassword" ) ).toBool() };
    uri.setParam( QStringLiteral( "savePassword" ), savePassword ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    if ( !savePassword )
    {
      uri.setPassword( QString() );
    }
  }

  const QStringList excludedSchemas = QgsMssqlConnection::excludedSchemasList( connName );
  if ( !excludedSchemas.isEmpty() )
    uri.setParam( QStringLiteral( "excludedSchemas" ), excludedSchemas.join( ',' ) );

  return uri;
}

QStringList QgsMssqlConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "MSSQL/connections" ) );
  return settings.childGroups();
}

QList<QgsVectorDataProvider::NativeType> QgsMssqlConnection::nativeTypes()
{
  return QList<QgsVectorDataProvider::NativeType>()
         // integer types
         << QgsVectorDataProvider::NativeType( QObject::tr( "8 Bytes Integer" ), QStringLiteral( "bigint" ), QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "4 Bytes Integer" ), QStringLiteral( "int" ), QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "2 Bytes Integer" ), QStringLiteral( "smallint" ), QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "1 Bytes Integer" ), QStringLiteral( "tinyint" ), QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (numeric)" ), QStringLiteral( "numeric" ), QMetaType::Type::Double, 1, 20, 0, 20 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (decimal)" ), QStringLiteral( "decimal" ), QMetaType::Type::Double, 1, 20, 0, 20 )

         // floating point
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (real)" ), QStringLiteral( "real" ), QMetaType::Type::Double )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (double)" ), QStringLiteral( "float" ), QMetaType::Type::Double )

         // date/time types
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), QStringLiteral( "date" ), QMetaType::Type::QDate, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), QStringLiteral( "time" ), QMetaType::Type::QTime, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), QStringLiteral( "datetime" ), QMetaType::Type::QDateTime, -1, -1, -1, -1 )

         // string types
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ), QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length unicode (nchar)" ), QStringLiteral( "nchar" ), QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length unicode (nvarchar)" ), QStringLiteral( "nvarchar" ), QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QMetaType::Type::QString )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length unicode (ntext)" ), QStringLiteral( "text" ), QMetaType::Type::QString );
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName )
{
  const QgsSettings settings;
  const QString databaseName = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/database" ) ).toString();

  return excludedSchemasList( connName, databaseName );
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName, const QString &database )
{
  const QgsSettings settings;
  const bool schemaFilteringEnabled = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/schemasFiltering" ) ).toBool();

  if ( schemaFilteringEnabled )
  {
    const QVariant schemaSettingsVariant = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/excludedSchemas" ) );

    if ( schemaSettingsVariant.userType() == QMetaType::Type::QVariantMap )
    {
      const QVariantMap schemaSettings = schemaSettingsVariant.toMap();
      if ( schemaSettings.contains( database ) && schemaSettings.value( database ).userType() == QMetaType::Type::QStringList )
        return schemaSettings.value( database ).toStringList();
    }
  }

  return QStringList();
}

void QgsMssqlConnection::setExcludedSchemasList( const QString &connName, const QStringList &excludedSchemas )
{
  const QgsSettings settings;

  const QString currentDatabaseName = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/database" ) ).toString();
  setExcludedSchemasList( connName, currentDatabaseName, excludedSchemas );
}

void QgsMssqlConnection::setExcludedSchemasList( const QString &connName, const QString &database, const QStringList &excludedSchemas )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/schemasFiltering" ), excludedSchemas.isEmpty() ? 0 : 1 );

  const QVariant schemaSettingsVariant = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/excludedSchemas" ) );
  QVariantMap schemaSettings = schemaSettingsVariant.toMap();
  schemaSettings.insert( database, excludedSchemas );
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/excludedSchemas" ), schemaSettings );
}

QString QgsMssqlConnection::buildQueryForTables( bool allowTablesWithNoGeometry, bool geometryColumnOnly, const QStringList &excludedSchemaList )
{
  QString notSelectedSchemas;
  if ( !excludedSchemaList.isEmpty() )
  {
    QStringList quotedSchemas;
    for ( const QString &sch : excludedSchemaList )
      quotedSchemas.append( QgsMssqlProvider::quotedValue( sch ) );
    notSelectedSchemas = quotedSchemas.join( ',' );
    notSelectedSchemas.prepend( QStringLiteral( "( " ) );
    notSelectedSchemas.append( QStringLiteral( " )" ) );
  }

  QString query( QStringLiteral( "SELECT " ) );
  if ( geometryColumnOnly )
  {
    query += QLatin1String( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0, coord_dimension FROM geometry_columns" );
    if ( !notSelectedSchemas.isEmpty() )
      query += QStringLiteral( " WHERE f_table_schema NOT IN %1" ).arg( notSelectedSchemas );
  }
  else
  {
    query += QStringLiteral( "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY', CASE when sys.objects.type = 'V' THEN 1 ELSE 0 END \n, 0"
                             "FROM sys.columns JOIN sys.types ON sys.columns.system_type_id = sys.types.system_type_id AND sys.columns.user_type_id = sys.types.user_type_id JOIN sys.objects ON sys.objects.object_id = sys.columns.object_id JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id \n"
                             "WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography') AND (sys.objects.type = 'U' OR sys.objects.type = 'V')" );
    if ( !notSelectedSchemas.isEmpty() )
      query += QStringLiteral( " AND (sys.schemas.name NOT IN %1)" ).arg( notSelectedSchemas );
  }

  if ( allowTablesWithNoGeometry )
  {
    query += QStringLiteral( " UNION ALL \n"
                             "SELECT sys.schemas.name, sys.objects.name, null, null, 'NONE', CASE when sys.objects.type = 'V' THEN 1 ELSE 0 END \n, 0"
                             "FROM  sys.objects JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id "
                             "WHERE NOT EXISTS (SELECT * FROM sys.columns sc1 JOIN sys.types ON sc1.system_type_id = sys.types.system_type_id WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography') AND sys.objects.object_id = sc1.object_id) AND (sys.objects.type = 'U' or sys.objects.type = 'V')" );
    if ( !notSelectedSchemas.isEmpty() )
      query += QStringLiteral( " AND sys.schemas.name NOT IN %1" ).arg( notSelectedSchemas );
  }

  return query;
}

QString QgsMssqlConnection::buildQueryForTables( const QString &connName, bool allowTablesWithNoGeometry )
{
  return buildQueryForTables( allowTablesWithNoGeometry, geometryColumnsOnly( connName ), excludedSchemasList( connName ) );
}

QString QgsMssqlConnection::buildQueryForTables( const QString &connName )
{
  return buildQueryForTables( allowGeometrylessTables( connName ), geometryColumnsOnly( connName ), excludedSchemasList( connName ) );
}

void QgsMssqlConnection::duplicateConnection( const QString &src, const QString &dst )
{
  const QString key( QStringLiteral( "/MSSQL/connections/" ) + src );
  const QString newKey( QStringLiteral( "/MSSQL/connections/" ) + dst );

  QgsSettings settings;
  settings.setValue( newKey + QStringLiteral( "/service" ), settings.value( key + QStringLiteral( "/service" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/host" ), settings.value( key + QStringLiteral( "/host" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/database" ), settings.value( key + QStringLiteral( "/database" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/username" ), settings.value( key + QStringLiteral( "/username" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/password" ), settings.value( key + QStringLiteral( "/password" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/saveUsername" ), settings.value( key + QStringLiteral( "/saveUsername" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/savePassword" ), settings.value( key + QStringLiteral( "/savePassword" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/geometryColumnsOnly" ), settings.value( key + QStringLiteral( "/geometryColumnsOnly" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/extentInGeometryColumns" ), settings.value( key + QStringLiteral( "/extentInGeometryColumns" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/primaryKeyInGeometryColumns" ), settings.value( key + QStringLiteral( "/primaryKeyInGeometryColumns" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/allowGeometrylessTables" ), settings.value( key + QStringLiteral( "/allowGeometrylessTables" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/estimatedMetadata" ), settings.value( key + QStringLiteral( "/estimatedMetadata" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/disableInvalidGeometryHandling" ), settings.value( key + QStringLiteral( "/disableInvalidGeometryHandling" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/schemasFiltering" ), settings.value( key + QStringLiteral( "/schemasFiltering" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/excludedSchemas" ), settings.value( key + QStringLiteral( "/excludedSchemas" ) ) );
  settings.sync();
}
