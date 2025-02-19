/***************************************************************************
  qgsmssqldatabase.cpp
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqldatabase.h"

#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsvariantutils.h"
#include "qgsmssqlprovider.h"
#include "qgsdbquerylog.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QFile>
#include <QThread>

constexpr int sMssqlDatabaseQueryLogFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define LoggedExec( query, sql ) execLogged( query, sql, QString( QString( __FILE__ ).mid( sMssqlDatabaseQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")" ) )


QRecursiveMutex QgsMssqlDatabase::sMutex;

QMap<QString, std::weak_ptr<QgsMssqlDatabase>> QgsMssqlDatabase::sConnections;


QString QgsMssqlDatabase::connectionName( const QString &service, const QString &host, const QString &database, bool transaction )
{
  QString connName;
  if ( service.isEmpty() )
  {
    if ( !host.isEmpty() )
      connName = host + '.';

    if ( database.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "QgsMssqlProvider database name not specified" ) );
      return QString();
    }

    connName += database;
  }
  else
    connName = service;

  if ( !transaction )
    connName += QStringLiteral( ":0x%1" ).arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) );
  else
    connName += ":transaction";
  return connName;
}


std::shared_ptr<QgsMssqlDatabase> QgsMssqlDatabase::connectDb( const QString &uri, bool transaction )
{
  QgsDataSourceUri dsUri( uri );
  return connectDb( dsUri, transaction );
}

std::shared_ptr<QgsMssqlDatabase> QgsMssqlDatabase::connectDb( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool transaction )
{
  QgsDataSourceUri uri;
  uri.setService( service );
  uri.setHost( host );
  uri.setDatabase( database );
  uri.setUsername( username );
  uri.setPassword( password );
  return connectDb( uri, transaction );
}

std::shared_ptr<QgsMssqlDatabase> QgsMssqlDatabase::connectDb( const QgsDataSourceUri &uri, bool transaction )
{
  // try to use existing conn or create a new one

  QMutexLocker locker( &sMutex );

  QString connName = connectionName( uri.service(), uri.host(), uri.database(), transaction );

  if ( sConnections.contains( connName ) && !sConnections[connName].expired() )
    return sConnections[connName].lock();

  QSqlDatabase db = getDatabase( uri.service(), uri.host(), uri.database(), uri.username(), uri.password(), transaction );

  std::shared_ptr<QgsMssqlDatabase> c( new QgsMssqlDatabase( db, uri, transaction ) );

  // we return connection even if it failed to open (because the error message may be useful)
  // but do not add it to connections as it is not useful
  if ( !c->isValid() )
    return c;

  sConnections[connName] = c;
  return c;
}

QgsMssqlDatabase::QgsMssqlDatabase( const QSqlDatabase &db, const QgsDataSourceUri &uri, bool transaction )
  : mUri( uri )
{
  mTransaction = transaction;
  mDB = db;

  if ( mTransaction )
  {
    mTransactionMutex.reset( new QRecursiveMutex );
  }

  if ( !mDB.isOpen() )
  {
    if ( !mDB.open() )
    {
      QgsDebugError( "Failed to open MSSQL database: " + mDB.lastError().text() );
    }
  }
}

bool QgsMssqlDatabase::execLogged( QSqlQuery &qry, const QString &sql, const QString &queryOrigin ) const
{
  QgsDatabaseQueryLogWrapper logWrapper { sql, mUri.uri(), QStringLiteral( "mssql" ), QStringLiteral( "QgsMssqlProvider" ), queryOrigin };
  const bool res { qry.exec( sql ) };
  if ( !res )
  {
    logWrapper.setError( qry.lastError().text() );
  }
  else
  {
    if ( qry.isSelect() )
    {
      logWrapper.setFetchedRows( qry.size() );
    }
    else
    {
      logWrapper.setFetchedRows( qry.numRowsAffected() );
    }
  }
  logWrapper.setQuery( qry.lastQuery() );
  return res;
}

QgsMssqlDatabase::~QgsMssqlDatabase()
{
  // close DB if it is open
  if ( mDB.isOpen() )
  {
    mDB.close();
  }
}

QSqlQuery QgsMssqlDatabase::createQuery()
{
  QSqlDatabase d = db();
  if ( !d.isOpen() )
  {
    QgsDebugError( "Creating query, but the database is not open!" );
  }
  return QSqlQuery( d );
}

QMetaType::Type QgsMssqlDatabase::decodeSqlType( const QString &sqlTypeName )
{
  QMetaType::Type type = QMetaType::Type::UnknownType;
  if ( sqlTypeName.startsWith( QLatin1String( "decimal" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "numeric" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "real" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "float" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Double;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "char" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "nchar" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "varchar" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "nvarchar" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "text" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "ntext" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "uniqueidentifier" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "smallint" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "int" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "bit" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "tinyint" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::Int;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "bigint" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::LongLong;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "binary" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "varbinary" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "image" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QByteArray;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "datetime" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "smalldatetime" ), Qt::CaseInsensitive ) || sqlTypeName.startsWith( QLatin1String( "datetime2" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDateTime;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "date" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QDate;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "timestamp" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QString;
  }
  else if ( sqlTypeName.startsWith( QLatin1String( "time" ), Qt::CaseInsensitive ) )
  {
    type = QMetaType::Type::QTime;
  }
  else
  {
    QgsDebugError( QStringLiteral( "Unknown field type: %1" ).arg( sqlTypeName ) );
    // Everything else just dumped as a string.
    type = QMetaType::Type::QString;
  }

  return type;
}


bool QgsMssqlDatabase::loadFields( FieldDetails &details, const QString &schema, const QString &tableName, QString &error )
{
  error.clear();

  bool isIdentity = false;
  details.attributeFields.clear();
  details.defaultValues.clear();
  details.computedColumns.clear();

  // get field spec
  QSqlQuery query = createQuery();
  query.setForwardOnly( true );

  const QString sql { QStringLiteral( "SELECT name FROM sys.columns WHERE is_computed = 1 AND object_id = OBJECT_ID('[%1].[%2]')" ).arg( schema, tableName ) };

  // Get computed columns which need to be ignored on insert or update.
  if ( !LoggedExec( query, sql ) )
  {
    error = query.lastError().text();
    return false;
  }

  while ( query.next() )
  {
    details.computedColumns.append( query.value( 0 ).toString() );
  }

  // Field has unique constraint
  QSet<QString> setColumnUnique;
  {
    const QString sql2 { QStringLiteral( "SELECT * FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS TC"
                                         " INNER JOIN INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE CC ON TC.CONSTRAINT_NAME = CC.CONSTRAINT_NAME"
                                         " WHERE TC.CONSTRAINT_SCHEMA = %1 AND TC.TABLE_NAME = %2 AND TC.CONSTRAINT_TYPE = 'unique'" )
                           .arg( QgsMssqlProvider::quotedValue( schema ), QgsMssqlProvider::quotedValue( tableName ) ) };
    if ( !LoggedExec( query, sql2 ) )
    {
      error = query.lastError().text();
      return false;
    }

    while ( query.next() )
    {
      setColumnUnique.insert( query.value( QStringLiteral( "COLUMN_NAME" ) ).toString() );
    }
  }

  const QString sql3 { QStringLiteral( "exec sp_columns @table_name = %1, @table_owner = %2" ).arg( QgsMssqlProvider::quotedValue( tableName ), QgsMssqlProvider::quotedValue( schema ) ) };
  if ( !LoggedExec( query, sql3 ) )
  {
    error = query.lastError().text();
    return false;
  }

  int i = 0;
  QStringList pkCandidates;
  while ( query.next() )
  {
    const QString colName = query.value( QStringLiteral( "COLUMN_NAME" ) ).toString();
    const QString sqlTypeName = query.value( QStringLiteral( "TYPE_NAME" ) ).toString();
    bool columnIsIdentity = false;

    // if we don't have an explicitly set geometry column name, and this is a geometry column, then use it
    // but if we DO have an explicitly set geometry column name, then load the other information if this is that column
    if ( ( details.geometryColumnName.isEmpty() && ( sqlTypeName == QLatin1String( "geometry" ) || sqlTypeName == QLatin1String( "geography" ) ) )
         || colName == details.geometryColumnName )
    {
      details.geometryColumnName = colName;
      details.geometryColumnType = sqlTypeName;
      details.isGeography = sqlTypeName == QLatin1String( "geography" );
    }
    else
    {
      const QMetaType::Type sqlType = decodeSqlType( sqlTypeName );
      if ( sqlTypeName == QLatin1String( "int identity" ) || sqlTypeName == QLatin1String( "bigint identity" ) )
      {
        details.primaryKeyType = PrimaryKeyType::Int;
        details.primaryKeyAttrs << details.attributeFields.size();
        columnIsIdentity = true;
        isIdentity = true;
      }
      else if ( sqlTypeName == QLatin1String( "int" ) || sqlTypeName == QLatin1String( "bigint" ) )
      {
        pkCandidates << colName;
      }

      QgsField field;
      if ( sqlType == QMetaType::Type::QString )
      {
        // Field length in chars is column 7 ("Length") of the sp_columns output,
        // except for uniqueidentifiers which must use column 6 ("Precision").
        int length = query.value( sqlTypeName.startsWith( QStringLiteral( "uniqueidentifier" ), Qt::CaseInsensitive ) ? 6 : 7 ).toInt();
        if ( sqlTypeName.startsWith( QLatin1Char( 'n' ) ) )
        {
          length = length / 2;
        }
        field = QgsField( colName, sqlType, sqlTypeName, length );
      }
      else if ( sqlType == QMetaType::Type::Double )
      {
        field = QgsField( colName, sqlType, sqlTypeName, query.value( QStringLiteral( "PRECISION" ) ).toInt(), sqlTypeName == QLatin1String( "decimal" ) || sqlTypeName == QLatin1String( "numeric" ) ? query.value( QStringLiteral( "SCALE" ) ).toInt() : -1 );
      }
      else if ( sqlType == QMetaType::Type::QDate || sqlType == QMetaType::Type::QDateTime || sqlType == QMetaType::Type::QTime )
      {
        field = QgsField( colName, sqlType, sqlTypeName, -1, -1 );
      }
      else
      {
        field = QgsField( colName, sqlType, sqlTypeName );
      }

      // Field nullable
      const bool nullable = query.value( QStringLiteral( "NULLABLE" ) ).toBool();

      // Set constraints
      QgsFieldConstraints constraints;
      if ( !nullable )
        constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      if ( setColumnUnique.contains( colName ) )
        constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      field.setConstraints( constraints );

      if ( columnIsIdentity )
      {
        field.setReadOnly( true );
      }

      details.attributeFields.append( field );

      // Default value
      if ( !QgsVariantUtils::isNull( query.value( QStringLiteral( "COLUMN_DEF" ) ) ) )
      {
        details.defaultValues.insert( i, query.value( QStringLiteral( "COLUMN_DEF" ) ).toString() );
      }
      else if ( columnIsIdentity )
      {
        // identity column types don't report a default value clause in the COLUMN_DEF attribute. So we need to fake
        // one, so that we can correctly indicate that the database is responsible for populating this column.
        details.defaultValues.insert( i, QStringLiteral( "Autogenerate" ) );
      }

      ++i;
    }
  }

  // get primary key
  if ( details.primaryKeyAttrs.isEmpty() )
  {
    query.clear();
    query.setForwardOnly( true );
    const QString sql4 { QStringLiteral( "exec sp_pkeys @table_name = %1, @table_owner = %2 " ).arg( QgsMssqlProvider::quotedValue( tableName ), QgsMssqlProvider::quotedValue( schema ) ) };
    if ( !LoggedExec( query, sql4 ) )
    {
      QgsDebugError( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
    }

    if ( query.isActive() )
    {
      details.primaryKeyType = PrimaryKeyType::Int;

      while ( query.next() )
      {
        const QString fidColName = query.value( 3 ).toString();
        const int idx = details.attributeFields.indexFromName( fidColName );
        const QgsField &fld = details.attributeFields.at( idx );

        if ( !details.primaryKeyAttrs.isEmpty() || ( fld.type() != QMetaType::Type::Int && fld.type() != QMetaType::Type::LongLong && ( fld.type() != QMetaType::Type::Double || fld.precision() != 0 ) ) )
          details.primaryKeyType = PrimaryKeyType::FidMap;

        details.primaryKeyAttrs << idx;
      }

      if ( details.primaryKeyAttrs.isEmpty() )
      {
        details.primaryKeyType = PrimaryKeyType::Unknown;
      }
    }
  }

  if ( details.primaryKeyAttrs.isEmpty() )
  {
    const auto constPkCandidates = pkCandidates;
    for ( const QString &pk : constPkCandidates )
    {
      query.clear();
      query.setForwardOnly( true );
      const QString sql5 { QStringLiteral( "select count(distinct [%1]), count([%1]) from [%2].[%3]" )
                             .arg( pk, schema, tableName ) };
      if ( !LoggedExec( query, sql5 ) )
      {
        QgsDebugError( QStringLiteral( "SQL:%1\n  Error:%2" ).arg( query.lastQuery(), query.lastError().text() ) );
      }

      if ( query.isActive() && query.next() && query.value( 0 ).toInt() == query.value( 1 ).toInt() )
      {
        details.primaryKeyType = PrimaryKeyType::Int;
        details.primaryKeyAttrs << details.attributeFields.indexFromName( pk );
        return true;
      }
    }
  }

  if ( details.primaryKeyAttrs.size() == 1 && !isIdentity )
  {
    // primary key has unique constraints
    QgsFieldConstraints constraints = details.attributeFields.at( details.primaryKeyAttrs[0] ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    details.attributeFields[details.primaryKeyAttrs[0]].setConstraints( constraints );
  }
  return true;
}


// -------------------


QSqlDatabase QgsMssqlDatabase::getDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool transaction )
{
  QSqlDatabase db;

  // while everything we use from QSqlDatabase here is thread safe, we need to ensure
  // that the connection cleanup on thread finalization happens in a predictable order
  QMutexLocker locker( &sMutex );

  const QString threadSafeConnectionName = connectionName( service, host, database, transaction );

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
      QObject::connect( QThread::currentThread(), &QThread::finished, QThread::currentThread(), [threadSafeConnectionName] {
        const QMutexLocker locker( &sMutex );
        QSqlDatabase::removeDatabase( threadSafeConnectionName ); }, Qt::DirectConnection );
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
#elif defined( Q_OS_MAC )
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
  // QgsDebugMsgLevel( connectionString, 2 );
  return db;
}
