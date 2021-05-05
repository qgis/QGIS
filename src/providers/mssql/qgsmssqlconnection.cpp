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
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsdatasourceuri.h"
#include <QSqlDatabase>
#include <QThread>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QCoreApplication>
#include <QFile>

int QgsMssqlConnection::sConnectionId = 0;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QMutex QgsMssqlConnection::sMutex { QMutex::Recursive };
#else
QRecursiveMutex QgsMssqlConnection::sMutex;
#endif

QSqlDatabase QgsMssqlConnection::getDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password )
{
  QSqlDatabase db;
  QString connectionName;

  // create a separate database connection for each feature source
  if ( service.isEmpty() )
  {
    if ( !host.isEmpty() )
      connectionName = host + '.';

    if ( database.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "QgsMssqlProvider database name not specified" ) );
      return db;
    }

    connectionName += QStringLiteral( "%1.%2" ).arg( database ).arg( sConnectionId++ );
  }
  else
    connectionName = service;

  // while everything we use from QSqlDatabase here is thread safe, we need to ensure
  // that the connection cleanup on thread finalization happens in a predictable order
  QMutexLocker locker( &sMutex );

  const QString threadSafeConnectionName = dbConnectionName( connectionName );

  if ( !QSqlDatabase::contains( threadSafeConnectionName ) )
  {
    db = QSqlDatabase::addDatabase( QStringLiteral( "QODBC" ), threadSafeConnectionName );
    db.setConnectOptions( QStringLiteral( "SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV" ) );

    // for background threads, remove database when current thread finishes
    if ( QThread::currentThread() != QCoreApplication::instance()->thread() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Scheduled auth db remove on thread close" ), 2 );

      // IMPORTANT - we use a direct connection here, because the database removal must happen immediately
      // when the thread finishes, and we cannot let this get queued on the main thread's event loop.
      // Otherwise, the QSqlDatabase's private data's thread gets reset immediately the QThread::finished,
      // and a subsequent call to QSqlDatabase::database with the same thread address (yep it happens, actually a lot)
      // triggers a condition in QSqlDatabase which detects the nullptr private thread data and returns an invalid database instead.
      // QSqlDatabase::removeDatabase is thread safe, so this is ok to do.
      QObject::connect( QThread::currentThread(), &QThread::finished, QThread::currentThread(), [threadSafeConnectionName]
      {
        QMutexLocker locker( &sMutex );
        QSqlDatabase::removeDatabase( threadSafeConnectionName );
      }, Qt::DirectConnection );
    }
  }
  else
  {
    db = QSqlDatabase::database( threadSafeConnectionName );
  }
  locker.unlock();

  db.setHostName( host );
  QString connectionString;
  if ( !service.isEmpty() )
  {
    // driver was specified explicitly
    connectionString = service;
  }
  else
  {
#ifdef Q_OS_WIN
    connectionString = "driver={SQL Server}";
#elif defined (Q_OS_MAC)
    QString freeTDSDriver( QCoreApplication::applicationDirPath().append( "/lib/libtdsodbc.so" ) );
    if ( QFile::exists( freeTDSDriver ) )
    {
      connectionString = QStringLiteral( "driver=%1;port=1433;TDS_Version=auto" ).arg( freeTDSDriver );
    }
    else
    {
      connectionString = QStringLiteral( "driver={FreeTDS};port=1433;TDS_Version=auto" );
    }
#else
    // It seems that FreeTDS driver by default uses an ancient TDS protocol version (4.2) to communicate with MS SQL
    // which was causing various data corruption errors, for example:
    // - truncating data from varchar columns to 255 chars - failing to read WKT for CRS
    // - truncating binary data to 4096 bytes (see @@TEXTSIZE) - failing to parse larger geometries
    // The added "TDS_Version=auto" should negotiate more recent version (manually setting e.g. 7.2 worked fine too)
    connectionString = QStringLiteral( "driver={FreeTDS};port=1433;TDS_Version=auto" );
#endif
  }

  if ( !host.isEmpty() )
    connectionString += ";server=" + host;

  if ( !database.isEmpty() )
    connectionString += ";database=" + database;

  if ( password.isEmpty() )
    connectionString += QLatin1String( ";trusted_connection=yes" );
  else
    connectionString += ";uid=" + username + ";pwd=" + password;

  if ( !username.isEmpty() )
    db.setUserName( username );

  if ( !password.isEmpty() )
    db.setPassword( password );

  db.setDatabaseName( connectionString );

  // only uncomment temporarily -- it can show connection password otherwise!
  // QgsDebugMsg( connectionString );
  return db;
}

bool QgsMssqlConnection::openDatabase( QSqlDatabase &db )
{
  if ( !db.isOpen() )
  {
    if ( !db.open() )
    {
      return false;
    }
  }
  return true;
}

bool QgsMssqlConnection::geometryColumnsOnly( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/geometryColumnsOnly", false ).toBool();
}

void QgsMssqlConnection::setGeometryColumnsOnly( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/geometryColumnsOnly", enabled );
}

bool QgsMssqlConnection::extentInGeometryColumns( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/extentInGeometryColumns", false ).toBool();
}

void QgsMssqlConnection::setExtentInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/extentInGeometryColumns", enabled );
}

bool QgsMssqlConnection::primaryKeyInGeometryColumns( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/primaryKeyInGeometryColumns", false ).toBool();
}

void QgsMssqlConnection::setPrimaryKeyInGeometryColumns( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/primaryKeyInGeometryColumns", enabled );
}

bool QgsMssqlConnection::allowGeometrylessTables( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/allowGeometrylessTables", false ).toBool();
}

void QgsMssqlConnection::setAllowGeometrylessTables( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/allowGeometrylessTables", enabled );
}

bool QgsMssqlConnection::useEstimatedMetadata( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/estimatedMetadata", false ).toBool();
}

void QgsMssqlConnection::setUseEstimatedMetadata( const QString &name, bool enabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/estimatedMetadata", enabled );
}

bool QgsMssqlConnection::isInvalidGeometryHandlingDisabled( const QString &name )
{
  QgsSettings settings;
  return settings.value( "/MSSQL/connections/" + name + "/disableInvalidGeometryHandling", false ).toBool();
}

void QgsMssqlConnection::setInvalidGeometryHandlingDisabled( const QString &name, bool disabled )
{
  QgsSettings settings;
  settings.setValue( "/MSSQL/connections/" + name + "/disableInvalidGeometryHandling", disabled );
}

bool QgsMssqlConnection::dropView( const QString &uri, QString *errorMessage )
{
  QgsDataSourceUri dsUri( uri );

  // connect to database
  QSqlDatabase db = getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !openDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery q = QSqlQuery( db );
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
  QgsDataSourceUri dsUri( uri );

  // connect to database
  QSqlDatabase db = getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !openDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  const QString sql = QString( "IF EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[%1].[%2]') AND type in (N'U')) DROP TABLE [%1].[%2]\n"
                               "DELETE FROM geometry_columns WHERE f_table_schema = '%1' AND f_table_name = '%2'" )
                      .arg( schema,
                            table );
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
  QgsDataSourceUri dsUri( uri );

  // connect to database
  QSqlDatabase db = getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );
  const QString schema = dsUri.schema();
  const QString table = dsUri.table();

  if ( !openDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery q = QSqlQuery( db );
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
  QgsDataSourceUri dsUri( uri );

  // connect to database
  QSqlDatabase db = getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  if ( !openDatabase( db ) )
  {
    if ( errorMessage )
      *errorMessage = db.lastError().text();
    return false;
  }

  QSqlQuery q = QSqlQuery( db );
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
  QgsDataSourceUri dsUri( uri );

// connect to database
  QSqlDatabase db = getDatabase( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password() );

  return schemas( db, errorMessage );
}

QStringList QgsMssqlConnection::schemas( QSqlDatabase &dataBase, QString *errorMessage )
{
  if ( !openDatabase( dataBase ) )
  {
    if ( errorMessage )
      *errorMessage = dataBase.lastError().text();
    return QStringList();
  }

  const QString sql = QStringLiteral( "select s.name as schema_name from sys.schemas s" );

  QSqlQuery q = QSqlQuery( dataBase );
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
  static QSet< QString > sSystemSchemas
  {
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
  QgsSettings settings;

  const QString key = "/MSSQL/connections/" + connName;

  const QString service = settings.value( key + "/service" ).toString();
  const QString host = settings.value( key + "/host" ).toString();
  const QString database = settings.value( key + "/database" ).toString();
  const QString username = settings.value( key + "/username" ).toString();
  const QString password = settings.value( key + "/password" ).toString();

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
    if ( ! saveUsername )
    {
      uri.setUsername( QString() );
    }
  }
  if ( settings.value( QStringLiteral( "savePassword" ) ).isValid() )
  {
    const bool savePassword { settings.value( QStringLiteral( "savePassword" ) ).toBool() };
    uri.setParam( QStringLiteral( "savePassword" ), savePassword ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    if ( ! savePassword )
    {
      uri.setPassword( QString() );
    }
  }

  QStringList excludedSchemas = QgsMssqlConnection::excludedSchemasList( connName );
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
         << QgsVectorDataProvider::NativeType( QObject::tr( "8 Bytes integer" ), QStringLiteral( "bigint" ), QVariant::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "4 Bytes integer" ), QStringLiteral( "int" ), QVariant::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "2 Bytes integer" ), QStringLiteral( "smallint" ), QVariant::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "1 Bytes integer" ), QStringLiteral( "tinyint" ), QVariant::Int )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

         // floating point
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal number (real)" ), QStringLiteral( "real" ), QVariant::Double )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Decimal number (double)" ), QStringLiteral( "float" ), QVariant::Double )

         // date/time types
         << QgsVectorDataProvider::NativeType( QObject::tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Date & Time" ), QStringLiteral( "datetime" ), QVariant::DateTime, -1, -1, -1, -1 )

         // string types
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QVariant::String, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ), QVariant::String, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, fixed length unicode (nchar)" ), QStringLiteral( "nchar" ), QVariant::String, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, limited variable length unicode (nvarchar)" ), QStringLiteral( "nvarchar" ), QVariant::String, 1, 255 )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String )
         << QgsVectorDataProvider::NativeType( QObject::tr( "Text, unlimited length unicode (ntext)" ), QStringLiteral( "text" ), QVariant::String )
         ;
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName )
{
  QgsSettings settings;
  QString databaseName = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/database" ) ).toString();

  return excludedSchemasList( connName, databaseName );
}

QStringList QgsMssqlConnection::excludedSchemasList( const QString &connName, const QString &database )
{
  QgsSettings settings;
  bool schemaFilteringEnabled = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/schemasFiltering" ) ).toBool();

  if ( schemaFilteringEnabled )
  {
    QVariant schemaSettingsVariant = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/excludedSchemas" ) );

    if ( schemaSettingsVariant.type() == QVariant::Map )
    {
      const QVariantMap schemaSettings = schemaSettingsVariant.toMap();
      if ( schemaSettings.contains( database ) && schemaSettings.value( database ).type() == QVariant::StringList )
        return schemaSettings.value( database ).toStringList();
    }
  }

  return QStringList();
}

void QgsMssqlConnection::setExcludedSchemasList( const QString &connName, const QStringList &excludedSchemas )
{
  QgsSettings settings;

  QString currentDatabaseName = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/database" ) ).toString();
  setExcludedSchemasList( connName, currentDatabaseName, excludedSchemas );
}

void QgsMssqlConnection::setExcludedSchemasList( const QString &connName, const QString &database, const QStringList &excludedSchemas )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/schemasFiltering" ), excludedSchemas.isEmpty() ? 0 : 1 );

  QVariant schemaSettingsVariant = settings.value( QStringLiteral( "/MSSQL/connections/" ) + connName + QStringLiteral( "/excludedSchemas" ) );
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
    query += QLatin1String( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type, 0 FROM geometry_columns" );
    if ( !notSelectedSchemas.isEmpty() )
      query += QStringLiteral( " WHERE f_table_schema NOT IN %1" ).arg( notSelectedSchemas );
  }
  else
  {
    query += QStringLiteral( "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY', CASE when sys.objects.type = 'V' THEN 1 ELSE 0 END \n"
                             "FROM sys.columns JOIN sys.types ON sys.columns.system_type_id = sys.types.system_type_id AND sys.columns.user_type_id = sys.types.user_type_id JOIN sys.objects ON sys.objects.object_id = sys.columns.object_id JOIN sys.schemas ON sys.objects.schema_id = sys.schemas.schema_id \n"
                             "WHERE (sys.types.name = 'geometry' OR sys.types.name = 'geography') AND (sys.objects.type = 'U' OR sys.objects.type = 'V')" );
    if ( !notSelectedSchemas.isEmpty() )
      query += QStringLiteral( " AND (sys.schemas.name NOT IN %1)" ).arg( notSelectedSchemas );
  }

  if ( allowTablesWithNoGeometry )
  {
    query += QStringLiteral( " UNION ALL \n"
                             "SELECT sys.schemas.name, sys.objects.name, null, null, 'NONE', case when sys.objects.type = 'V' THEN 1 ELSE 0 END \n"
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

QString QgsMssqlConnection::dbConnectionName( const QString &name )
{
  // Starting with Qt 5.11, sharing the same connection between threads is not allowed.
  // We use a dedicated connection for each thread requiring access to the database,
  // using the thread address as connection name.
  return QStringLiteral( "%1:0x%2" ).arg( name ).arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) );
}
