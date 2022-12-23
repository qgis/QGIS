/***************************************************************************
  qgsdbquerylog.cpp
  ------------
  Date                 : October 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbquerylog.h"
#include "qgsapplication.h"
#include <QDateTime>

//
// QgsDatabaseQueryLogEntry
//

QAtomicInt QgsDatabaseQueryLogEntry::sQueryId = 0;

QgsDatabaseQueryLogEntry::QgsDatabaseQueryLogEntry( const QString &query )
  : queryId( ++sQueryId )
  , query( query )
  , startedTime( QDateTime::currentMSecsSinceEpoch() )
{}


//
// QgsDatabaseQueryLog
//

bool QgsDatabaseQueryLog::sEnabled = false;

QgsDatabaseQueryLog::QgsDatabaseQueryLog( QObject *parent )
  : QObject( parent )
{

}

void QgsDatabaseQueryLog::log( const QgsDatabaseQueryLogEntry &query )
{
  if ( !sEnabled )
    return;

  QMetaObject::invokeMethod( QgsApplication::databaseQueryLog(), "queryStartedPrivate", Qt::QueuedConnection, Q_ARG( QgsDatabaseQueryLogEntry, query ) );
}

void QgsDatabaseQueryLog::finished( const QgsDatabaseQueryLogEntry &query )
{
  if ( !sEnabled )
    return;

  // record time of completion
  QgsDatabaseQueryLogEntry finishedQuery = query;
  finishedQuery.finishedTime = QDateTime::currentMSecsSinceEpoch();

  QMetaObject::invokeMethod( QgsApplication::databaseQueryLog(), "queryFinishedPrivate", Qt::QueuedConnection, Q_ARG( QgsDatabaseQueryLogEntry, finishedQuery ) );
}

void QgsDatabaseQueryLog::queryStartedPrivate( const QgsDatabaseQueryLogEntry &query )
{
  QgsDebugMsgLevel( query.query, 2 );
  emit queryStarted( query );
}

void QgsDatabaseQueryLog::queryFinishedPrivate( const QgsDatabaseQueryLogEntry &query )
{
  QgsDebugMsgLevel( query.query, 2 );
  emit queryFinished( query );
}

