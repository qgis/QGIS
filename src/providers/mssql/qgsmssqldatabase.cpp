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

#include <QCoreApplication>
#include <QtDebug>
#include <QFile>
#include <QThread>


#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QMutex QgsMssqlDatabase::sMutex { QMutex::Recursive };
#else
QRecursiveMutex QgsMssqlDatabase::sMutex;
#endif

QMap<QString, std::weak_ptr<QgsMssqlDatabase> > QgsMssqlDatabase::sConnections;


QString QgsMssqlDatabase::connectionName( const QString &service, const QString &host, const QString &database, bool transaction )
{
  QString connName;
  if ( service.isEmpty() )
  {
    if ( !host.isEmpty() )
      connName = host + '.';

    if ( database.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "QgsMssqlProvider database name not specified" ) );
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
  return connectDb( dsUri.service(), dsUri.host(), dsUri.database(), dsUri.username(), dsUri.password(), transaction );
}

std::shared_ptr<QgsMssqlDatabase> QgsMssqlDatabase::connectDb( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool transaction )
{
  // try to use existing conn or create a new one

  QMutexLocker locker( &sMutex );

  QString connName = connectionName( service, host, database, transaction );

  if ( sConnections.contains( connName ) && !sConnections[connName].expired() )
    return sConnections[connName].lock();

  QSqlDatabase db = getDatabase( service, host, database, username, password, transaction );

  std::shared_ptr<QgsMssqlDatabase> c( new QgsMssqlDatabase( db, transaction ) );

  // we return connection even if it failed to open (because the error message may be useful)
  // but do not add it to connections as it is not useful
  if ( !c->isValid() )
    return c;

  sConnections[connName] = c;
  return c;
}

QgsMssqlDatabase::QgsMssqlDatabase( const QSqlDatabase &db, bool transaction )
{
  mTransaction = transaction;
  mDB = db;

  if ( mTransaction )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    mTransactionMutex.reset( new QMutex { QMutex::Recursive } );
#else
    mTransactionMutex.reset( new QRecursiveMutex );
#endif
  }

  if ( !mDB.isOpen() )
  {
    if ( !mDB.open() )
    {
      QgsDebugMsg( "Failed to open MSSQL database: " + mDB.lastError().text() );
    }
  }
}

QgsMssqlDatabase::~QgsMssqlDatabase()
{
  // close DB if it is open
  if ( mDB.isOpen() )
  {
    mDB.close();
  }
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
      QObject::connect( QThread::currentThread(), &QThread::finished, QThread::currentThread(), [threadSafeConnectionName]
      {
        const QMutexLocker locker( &sMutex );
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
