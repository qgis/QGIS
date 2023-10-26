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
#include "qgsdatasourceuri.h"
#include "qgscredentials.h"
#include "qgslogger.h"
#include "qgspostgresconn.h"

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <sys/select.h>
#endif

extern "C"
{
#include <libpq-fe.h>
}

std::unique_ptr< QgsPostgresListener > QgsPostgresListener::create( const QString &connString )
{
  std::unique_ptr< QgsPostgresListener > res( new QgsPostgresListener( connString ) );
  QgsDebugMsgLevel( QStringLiteral( "starting notification listener" ), 2 );

  res->start();
  return res;
}

QgsPostgresListener::QgsPostgresListener( const QString &connString )
{
  mConn = QgsPostgresConn::connectDb( connString, true, false );
  if ( mConn )
  {
    mConn->moveToThread( this );

    QgsPostgresResult result( mConn->LoggedPQexec( "QgsPostgresListener", QStringLiteral( "LISTEN qgis" ) ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugError( QStringLiteral( "error in listen" ) );

      mConn->unref();
      mConn = nullptr;
    }
  }
}

QgsPostgresListener::~QgsPostgresListener()
{
  mStop = true;
  QgsDebugMsgLevel( QStringLiteral( "stopping the loop" ), 2 );
  wait();
  QgsDebugMsgLevel( QStringLiteral( "notification listener stopped" ), 2 );

  if ( mConn )
    mConn->unref();
}

void QgsPostgresListener::run()
{
  if ( !mConn )
  {
    QgsDebugError( QStringLiteral( "error in listen" ) );
    return;
  }

  const int sock = PQsocket( mConn->pgConnection() );
  if ( sock < 0 )
  {
    QgsDebugError( QStringLiteral( "error in socket" ) );
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
      QgsDebugError( QStringLiteral( "error in select" ) );
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
      QgsDebugMsgLevel( QStringLiteral( "stop from main thread" ), 2 );
      break;
    }
  }
}
