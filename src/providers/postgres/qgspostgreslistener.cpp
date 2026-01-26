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

#include "moc_qgspostgreslistener.cpp"

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <sys/select.h>
#endif

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

  const int sock = PQsocket( mConn->pgConnection() );
  if ( sock < 0 )
  {
    QgsDebugError( u"error in socket"_s );
    return;
  }

  PGconn *pgconn = mConn->pgConnection();
  forever
  {
    fd_set input_mask;
    FD_ZERO( &input_mask );
    FD_SET( sock, &input_mask );

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if ( select( sock + 1, &input_mask, nullptr, nullptr, &timeout ) < 0 )
    {
      QgsDebugError( u"error in select"_s );
      break;
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
