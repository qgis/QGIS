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

#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmssqldatabase.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqlutils.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"

#include <QFile>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

bool QgsMssqlConnection::geometryColumnsOnly( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/geometryColumnsOnly"_s, false ).toBool();
}

void QgsMssqlConnection::setGeometryColumnsOnly( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/geometryColumnsOnly"_s, enabled );
}

bool QgsMssqlConnection::extentInGeometryColumns( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/extentInGeometryColumns"_s, false ).toBool();
}

void QgsMssqlConnection::setExtentInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/extentInGeometryColumns"_s, enabled );
}

bool QgsMssqlConnection::primaryKeyInGeometryColumns( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/primaryKeyInGeometryColumns"_s, false ).toBool();
}

void QgsMssqlConnection::setPrimaryKeyInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/primaryKeyInGeometryColumns"_s, enabled );
}

bool QgsMssqlConnection::allowGeometrylessTables( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/allowGeometrylessTables"_s, false ).toBool();
}

void QgsMssqlConnection::setAllowGeometrylessTables( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/allowGeometrylessTables"_s, enabled );
}

bool QgsMssqlConnection::useEstimatedMetadata( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/estimatedMetadata"_s, false ).toBool();
}

void QgsMssqlConnection::setUseEstimatedMetadata( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/estimatedMetadata"_s, enabled );
}

bool QgsMssqlConnection::isInvalidGeometryHandlingDisabled( const QString &name )
{
  const QgsSettings settings;
  return settings.value( u"/MSSQL/connections/"_s + name + u"/disableInvalidGeometryHandling"_s, false ).toBool();
}

void QgsMssqlConnection::setInvalidGeometryHandlingDisabled( const QString &name, bool disabled )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + name + u"/disableInvalidGeometryHandling"_s, disabled );
}

bool QgsMssqlConnection::dropView( const QString &uri, QString *errorMessage )
{
  const QgsDataSourceUri dsUri( uri );

  // connect to database
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );
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
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );
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
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );
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
  const QString sql = u"TRUNCATE TABLE [%1].[%2]"_s.arg( schema, table );
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
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );

  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return false;
  }

  QSqlQuery q = QSqlQuery( db->db() );
  q.setForwardOnly( true );
  const QString sql = u"CREATE SCHEMA [%1]"_s.arg( schemaName );
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
  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( dsUri );

  return schemas( std::move( db ), errorMessage );
}

QStringList QgsMssqlConnection::schemas( std::shared_ptr<QgsMssqlDatabase> db, QString *errorMessage )
{
  if ( !db->isValid() )
  {
    if ( errorMessage )
      *errorMessage = db->errorText();
    return QStringList();
  }

  const QString sql = u"select s.name as schema_name from sys.schemas s"_s;

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
    u"db_owner"_s,
    u"db_securityadmin"_s,
    u"db_accessadmin"_s,
    u"db_backupoperator"_s,
    u"db_ddladmin"_s,
    u"db_datawriter"_s,
    u"db_datareader"_s,
    u"db_denydatawriter"_s,
    u"db_denydatareader"_s,
    u"INFORMATION_SCHEMA"_s,
    u"sys"_s
  };

  return sSystemSchemas.contains( schema );
}

QgsDataSourceUri QgsMssqlConnection::connUri( const QString &connName )
{
  const QgsSettings settings;

  const QString key = u"/MSSQL/connections/"_s + connName;

  const QString service = settings.value( key + u"/service"_s ).toString();
  const QString host = settings.value( key + u"/host"_s ).toString();
  const QString database = settings.value( key + u"/database"_s ).toString();
  const QString username = settings.value( key + u"/username"_s ).toString();
  const QString password = settings.value( key + u"/password"_s ).toString();

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

  uri.setParam( u"geometryColumnsOnly"_s, useGeometryColumns ? u"true"_s : u"false"_s );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  uri.setParam( u"allowGeometrylessTables"_s, allowGeometrylessTables ? u"true"_s : u"false"_s );
  uri.setParam( u"disableInvalidGeometryHandling"_s, disableGeometryHandling ? u"true"_s : u"false"_s );

  if ( settings.value( u"saveUsername"_s ).isValid() )
  {
    const bool saveUsername { settings.value( u"saveUsername"_s ).toBool() };
    uri.setParam( u"saveUsername"_s, saveUsername ? u"true"_s : u"false"_s );
    if ( !saveUsername )
    {
      uri.setUsername( QString() );
    }
  }
  if ( settings.value( u"savePassword"_s ).isValid() )
  {
    const bool savePassword { settings.value( u"savePassword"_s ).toBool() };
    uri.setParam( u"savePassword"_s, savePassword ? u"true"_s : u"false"_s );
    if ( !savePassword )
    {
      uri.setPassword( QString() );
    }
  }

  const QStringList excludedSchemas = QgsMssqlConnection::excludedSchemasList( connName );
  if ( !excludedSchemas.isEmpty() )
    uri.setParam( u"excludedSchemas"_s, excludedSchemas.join( ',' ) );

  return uri;
}

QStringList QgsMssqlConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( u"MSSQL/connections"_s );
  return settings.childGroups();
}

QList<QgsVectorDataProvider::NativeType> QgsMssqlConnection::nativeTypes()
{
  return QList<QgsVectorDataProvider::NativeType>()
         // integer types
         << QgsVectorDataProvider::NativeType( QObject::tr( "8 Bytes Integer (bigint)" ), u"bigint"_s, QMetaType::Type::LongLong )
         << QgsVectorDataProvider::NativeType( QObject::tr( "4 Bytes Integer (int)" ), u"int"_s, QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "2 Bytes Integer (smallint)" ), u"smallint"_s, QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "1 Bytes Integer (tinyint)" ), u"tinyint"_s, QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (numeric)" ), u"numeric"_s, QMetaType::Type::Double, 1, 20, 0, 20 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (decimal)" ), u"decimal"_s, QMetaType::Type::Double, 1, 20, 0, 20 )

         // floating point
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (real)" ), u"real"_s, QMetaType::Type::Double )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal Number (double)" ), u"float"_s, QMetaType::Type::Double )

         // date/time types
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), u"date"_s, QMetaType::Type::QDate, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), u"time"_s, QMetaType::Type::QTime, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), u"datetime"_s, QMetaType::Type::QDateTime, -1, -1, -1, -1 )

         // string types
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length (char)" ), u"char"_s, QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length (varchar)" ), u"varchar"_s, QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length unicode (nchar)" ), u"nchar"_s, QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length unicode (nvarchar)" ), u"nvarchar"_s, QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length (text)" ), u"text"_s, QMetaType::Type::QString )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length unicode (ntext)" ), u"text"_s, QMetaType::Type::QString );
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName )
{
  const QgsSettings settings;
  const QString databaseName = settings.value( u"/MSSQL/connections/"_s + connName + u"/database"_s ).toString();

  return excludedSchemasList( connName, databaseName );
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName, const QString &database )
{
  const QgsSettings settings;
  const bool schemaFilteringEnabled = settings.value( u"/MSSQL/connections/"_s + connName + u"/schemasFiltering"_s ).toBool();

  if ( schemaFilteringEnabled )
  {
    const QVariant schemaSettingsVariant = settings.value( u"/MSSQL/connections/"_s + connName + u"/excludedSchemas"_s );

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

  const QString currentDatabaseName = settings.value( u"/MSSQL/connections/"_s + connName + u"/database"_s ).toString();
  setExcludedSchemasList( connName, currentDatabaseName, excludedSchemas );
}

void QgsMssqlConnection::setExcludedSchemasList( const QString &connName, const QString &database, const QStringList &excludedSchemas )
{
  QgsSettings settings;
  settings.setValue( u"/MSSQL/connections/"_s + connName + u"/schemasFiltering"_s, excludedSchemas.isEmpty() ? 0 : 1 );

  const QVariant schemaSettingsVariant = settings.value( u"/MSSQL/connections/"_s + connName + u"/excludedSchemas"_s );
  QVariantMap schemaSettings = schemaSettingsVariant.toMap();
  schemaSettings.insert( database, excludedSchemas );
  settings.setValue( u"/MSSQL/connections/"_s + connName + u"/excludedSchemas"_s, schemaSettings );
}

QString QgsMssqlConnection::buildQueryForTables( bool allowTablesWithNoGeometry, bool geometryColumnOnly, const QStringList &excludedSchemaList )
{
  QString notSelectedSchemas;
  if ( !excludedSchemaList.isEmpty() )
  {
    QStringList quotedSchemas;
    for ( const QString &sch : excludedSchemaList )
      quotedSchemas.append( QgsMssqlUtils::quotedValue( sch ) );
    notSelectedSchemas = quotedSchemas.join( ',' );
    notSelectedSchemas.prepend( u"( "_s );
    notSelectedSchemas.append( u" )"_s );
  }

  QString query( u"SELECT "_s );
  if ( geometryColumnOnly )
  {
    query += "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0, coord_dimension FROM geometry_columns"_L1;
    if ( !notSelectedSchemas.isEmpty() )
      query += u" WHERE f_table_schema NOT IN %1"_s.arg( notSelectedSchemas );
  }
  else
  {
    query += QStringLiteral( "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY', CASE when sys.objects.type = 'V' THEN 1 ELSE 0 END \n, 0"
                             "FROM sys.columns JOIN sys.types ON sys.columns.system_type_id = sys.types.system_type_id AND sys.columns.user_type_id = sys.types.user_type_id JOIN sys.objects ON sys.objects.object_id = sys.columns.object_id JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id \n"
                             "WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography') AND (sys.objects.type = 'U' OR sys.objects.type = 'V')" );
    if ( !notSelectedSchemas.isEmpty() )
      query += u" AND (sys.schemas.name NOT IN %1)"_s.arg( notSelectedSchemas );
  }

  if ( allowTablesWithNoGeometry )
  {
    query += QStringLiteral( " UNION ALL \n"
                             "SELECT sys.schemas.name, sys.objects.name, null, null, 'NONE', CASE when sys.objects.type = 'V' THEN 1 ELSE 0 END \n, 0"
                             "FROM  sys.objects JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id "
                             "WHERE NOT EXISTS (SELECT * FROM sys.columns sc1 JOIN sys.types ON sc1.system_type_id = sys.types.system_type_id WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography') AND sys.objects.object_id = sc1.object_id) AND (sys.objects.type = 'U' or sys.objects.type = 'V')" );
    if ( !notSelectedSchemas.isEmpty() )
      query += u" AND sys.schemas.name NOT IN %1"_s.arg( notSelectedSchemas );
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
  const QString key( u"/MSSQL/connections/"_s + src );
  const QString newKey( u"/MSSQL/connections/"_s + dst );

  QgsSettings settings;
  settings.setValue( newKey + u"/service"_s, settings.value( key + u"/service"_s ).toString() );
  settings.setValue( newKey + u"/host"_s, settings.value( key + u"/host"_s ).toString() );
  settings.setValue( newKey + u"/database"_s, settings.value( key + u"/database"_s ).toString() );
  settings.setValue( newKey + u"/username"_s, settings.value( key + u"/username"_s ).toString() );
  settings.setValue( newKey + u"/password"_s, settings.value( key + u"/password"_s ).toString() );
  settings.setValue( newKey + u"/saveUsername"_s, settings.value( key + u"/saveUsername"_s ).toString() );
  settings.setValue( newKey + u"/savePassword"_s, settings.value( key + u"/savePassword"_s ).toString() );
  settings.setValue( newKey + u"/geometryColumnsOnly"_s, settings.value( key + u"/geometryColumnsOnly"_s ).toBool() );
  settings.setValue( newKey + u"/extentInGeometryColumns"_s, settings.value( key + u"/extentInGeometryColumns"_s ).toBool() );
  settings.setValue( newKey + u"/primaryKeyInGeometryColumns"_s, settings.value( key + u"/primaryKeyInGeometryColumns"_s ).toBool() );
  settings.setValue( newKey + u"/allowGeometrylessTables"_s, settings.value( key + u"/allowGeometrylessTables"_s ).toBool() );
  settings.setValue( newKey + u"/estimatedMetadata"_s, settings.value( key + u"/estimatedMetadata"_s ).toBool() );
  settings.setValue( newKey + u"/disableInvalidGeometryHandling"_s, settings.value( key + u"/disableInvalidGeometryHandling"_s ).toBool() );
  settings.setValue( newKey + u"/schemasFiltering"_s, settings.value( key + u"/schemasFiltering"_s ).toBool() );
  settings.setValue( newKey + u"/excludedSchemas"_s, settings.value( key + u"/excludedSchemas"_s ) );
  settings.sync();
}
