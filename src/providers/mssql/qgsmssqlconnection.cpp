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
#include "qgslogger.h"
#include <QSqlDatabase>
#include <QThread>

int QgsMssqlConnection::sConnectionId = 0;

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

  const QString threadSafeConnectionName = dbConnectionName( connectionName );
  if ( !QSqlDatabase::contains( threadSafeConnectionName ) )
  {
    db = QSqlDatabase::addDatabase( QStringLiteral( "QODBC" ), threadSafeConnectionName );
    db.setConnectOptions( QStringLiteral( "SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV" ) );
  }
  else
    db = QSqlDatabase::database( threadSafeConnectionName );

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
#else
    connectionString = QStringLiteral( "driver={FreeTDS};port=1433" );
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

QString QgsMssqlConnection::dbConnectionName( const QString &name )
{
  // Starting with Qt 5.11, sharing the same connection between threads is not allowed.
  // We use a dedicated connection for each thread requiring access to the database,
  // using the thread address as connection name.
  const QString threadAddress = QStringLiteral( ":0x%1" ).arg( QString::number( reinterpret_cast< quintptr >( QThread::currentThread() ), 16 ) );
  return name + threadAddress;
}
