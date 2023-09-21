/***************************************************************************
    qgsspatialiteconnpool.cpp
    ---------------------
    begin                : February 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMutex>
#include <QMutexLocker>

#include "qgsspatialiteconnpool.h"

void QgsSpatiaLiteConnPoolGroup::connectionCreate( const QString &connectionInfo, QgsSqliteHandle *&connection )
{
  connection = QgsSqliteHandle::openDb( connectionInfo, false );
}

void QgsSpatiaLiteConnPoolGroup::connectionDestroy( QgsSqliteHandle *connection )
{
  QgsSqliteHandle::closeDb( connection );  // will delete itself
}

void QgsSpatiaLiteConnPoolGroup::invalidateConnection( QgsSqliteHandle *connection )
{
  /* Invalidation is used in particular by the WFS provider that uses a */
  /* temporary SpatiaLite DB and want to delete it at some point. For that */
  /* it must invalidate all handles pointing to it */
  connection->invalidate();
}

bool QgsSpatiaLiteConnPoolGroup::connectionIsValid( QgsSqliteHandle *connection )
{
  return connection->isValid();
}


QgsSpatiaLiteConnPool *QgsSpatiaLiteConnPool::sInstance = nullptr;

QgsSpatiaLiteConnPool *QgsSpatiaLiteConnPool::instance()
{
  if ( ! sInstance )
  {
    static QMutex sMutex;
    QMutexLocker locker( &sMutex );
    // cppcheck-suppress identicalInnerCondition
    if ( ! sInstance )
    {
      sInstance = new QgsSpatiaLiteConnPool();
    }
  }
  return sInstance;
}

// static public
void QgsSpatiaLiteConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QString QgsSpatiaLiteConnPool::connectionToName( QgsSqliteHandle *connection )
{
  return connection->dbPath();
}
