/***************************************************************************
  qgspostgreslistener.cpp  -  Listen to postgres NOTIFY
                             -------------------
    begin                : Sept 11, 2017
    copyright            : (C) 2017 by Vincent Mora
    email                : vincent dor mora at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgreslistener.h"

#include "qgscredentials.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgspostgresconn.h"

#include <QString>

#include "moc_qgspostgreslistener.cpp"

using namespace Qt::StringLiterals;

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <sys/select.h>
#endif

#include <cerrno>

extern "C"
{
#include <libpq-fe.h>
}

std::unique_ptr<QgsPostgresListener> QgsPostgresListener::create( const QString &connString )
{
  auto res = std::make_unique<QgsPostgresListener>( connString );
  QgsDebugMsgLevel( u"starting notification listener"_s, 2 );

  res->start();
  return res;
}

QgsPostgresListener::QgsPostgresListener( const QString &connString )
{
  mConn = QgsPostgresConn::connectDb( connString, true, false );
  if ( mConn )
  {
    mConn->moveToThread( this );

    QgsPostgresResult result( mConn->LoggedPQexec( "QgsPostgresListener", u"LISTEN qgis"_s ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugError( u"error in listen"_s );

      mConn->unref();
      mConn = nullptr;
    }
  }
}

QgsPostgresListener::~QgsPostgresListener()
{
  mStop = true;
  QgsDebugMsgLevel( u"stopping the loop"_s, 2 );
  wait();
  QgsDebugMsgLevel( u"notification listener stopped"_s, 2 );

  if ( mConn )
    mConn->unref();
}

void QgsPostgresListener::run()
{
  if ( !mConn )
  {
    QgsDebugError( u"error in listen"_s );
    return;
  }

  PGconn *pgconn = mConn->pgConnection();
  int sock = PQsocket( pgconn );
  if ( sock < 0 )
  {
    QgsDebugError( u"error in socket"_s );
    return;
  }

  // The connection is usable only if libpq reports it OK, the fd is valid
  // and small enough for select() (FD_SET would be undefined behaviour
  // otherwise).
  auto connectionUsable = [ & ]() -> bool {
    return PQstatus( pgconn ) == CONNECTION_OK && sock >= 0 && sock < FD_SETSIZE;
  };

  // Re-establish the connection and re-subscribe to the "qgis" channel.
  auto resetConnection = [ & ]() -> bool {
    ::PQreset( pgconn );
    if ( PQstatus( pgconn ) != CONNECTION_OK )
      return false;

    QgsPostgresResult result( mConn->LoggedPQexec( "QgsPostgresListener", u"LISTEN qgis"_s ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugError( u"error in listen after reset"_s );
      return false;
    }

    // the socket fd may have changed after the reset
    sock = PQsocket( pgconn );
    return sock >= 0;
  };

  forever
  {
    if ( !connectionUsable() )
    {
      QgsDebugMsgLevel( u"notification listener: connection lost, resetting"_s, 2 );
      if ( !resetConnection() || !connectionUsable() )
      {
        // Network probably still down: PQreset() honours connect_timeout,
        // so back off a bit instead of spinning
        msleep( 1000 );
        continue;
      }
      QgsDebugMsgLevel( u"notification listener: reconnected and re-listening"_s, 2 );
    }

    fd_set input_mask;
    FD_ZERO( &input_mask );
    FD_SET( sock, &input_mask );

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if ( select( sock + 1, &input_mask, nullptr, nullptr, &timeout ) < 0 )
    {
      // EINTR is harmless, just loop again
      if ( errno == EINTR )
        continue;
      // The kernel may have destroyed the socket under our feet without
      // telling libpq: on Android, switching networks (wifi/cellular/
      // airplane mode) kills all TCP sockets and the fd becomes invalid,
      // so select() fails with EBADF while PQstatus() still reports
      // CONNECTION_OK. Exiting the loop here would leave notifications
      // broken until the project is reloaded, so force a reset instead.
      QgsDebugMsgLevel( u"notification listener: select error, forcing reset"_s, 2 );
      sock = -1;
      continue;
    }

    PQconsumeInput( pgconn );
    PGnotify *n = PQnotifies( pgconn );
    if ( n )
    {
      const QString msg( n->extra );
      emit notify( msg );
      QgsDebugMsgLevel( "notify " + msg, 2 );
      PQfreemem( n );
    }

    if ( mStop )
    {
      QgsDebugMsgLevel( u"stop from main thread"_s, 2 );
      break;
    }
  }
}
