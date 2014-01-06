/***************************************************************************
    qgspostgresconnpool.cpp
    ---------------------
    begin                : January 2014
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

#include "qgspostgresconnpool.h"

#include "qgspostgresconn.h"

#include <QCoreApplication>

#define POSTGRES_MAX_CONCURRENT_CONNS      4
#define POSTGRES_CONN_EXPIRATION          60    // in seconds



QgsPostgresConnPoolGroup::QgsPostgresConnPoolGroup( const QString& ci )
  : connInfo( ci )
  , sem( POSTGRES_MAX_CONCURRENT_CONNS )
  , expirationTimer( this )
{
  // just to make sure the object belongs to main thread and thus will get events
  moveToThread( qApp->thread() );

  expirationTimer.setInterval( POSTGRES_CONN_EXPIRATION * 1000 );
  connect( &expirationTimer, SIGNAL( timeout() ), this, SLOT( handleConnectionExpired() ) );
}


QgsPostgresConnPoolGroup::~QgsPostgresConnPoolGroup()
{
  foreach ( Item item, conns )
    item.c->disconnect();
}


QgsPostgresConn* QgsPostgresConnPoolGroup::acquire()
{
  // we are going to acquire a resource - if no resource is available, we will block here
  sem.acquire();

  // quick (preferred) way - use cached connection
  {
    QMutexLocker locker( &connMutex );

    if ( !conns.isEmpty() )
    {
      Item i = conns.pop();

      // no need to run if nothing can expire
      if ( conns.isEmpty() )
        expirationTimer.stop();

      return i.c;
    }
  }

  QgsPostgresConn* c = QgsPostgresConn::connectDb( connInfo, true, false ); // TODO: read-only
  if ( !c )
  {
    // we didn't get connection for some reason, so release the lock
    sem.release();
    return 0;
  }

  return c;
}


void QgsPostgresConnPoolGroup::release( QgsPostgresConn* conn )
{
  connMutex.lock();
  Item i;
  i.c = conn;
  i.lastUsedTime = QTime::currentTime();
  conns.push( i );

  if ( !expirationTimer.isActive() )
    expirationTimer.start();

  connMutex.unlock();

  sem.release(); // this can unlock a thread waiting in acquire()
}


void QgsPostgresConnPoolGroup::handleConnectionExpired()
{
  connMutex.lock();

  QTime now = QTime::currentTime();

  // what connections have expired?
  QList<int> toDelete;
  for ( int i = 0; i < conns.count(); ++i )
  {
    if ( conns.at( i ).lastUsedTime.secsTo( now ) >= POSTGRES_CONN_EXPIRATION )
      toDelete.append( i );
  }

  // delete expired connections
  for ( int j = toDelete.count() - 1; j >= 0; --j )
  {
    int index = toDelete[j];
    conns[index].c->disconnect();
    conns.remove( index );
  }

  if ( conns.isEmpty() )
    expirationTimer.stop();

  connMutex.unlock();
}


// ----


QgsPostgresConnPool* QgsPostgresConnPool::mInstance = 0;


QgsPostgresConnPool* QgsPostgresConnPool::instance()
{
  if ( !mInstance )
    mInstance = new QgsPostgresConnPool;
  return mInstance;
}


QgsPostgresConn* QgsPostgresConnPool::acquireConnection( const QString& connInfo )
{
  mMutex.lock();
  QgsPostgresConnPoolGroups::iterator it = mGroups.find( connInfo );
  if ( it == mGroups.end() )
  {
    it = mGroups.insert( connInfo, new QgsPostgresConnPoolGroup( connInfo ) );
  }
  QgsPostgresConnPoolGroup* group = *it;
  mMutex.unlock();

  return group->acquire();
}


void QgsPostgresConnPool::releaseConnection( QgsPostgresConn* conn )
{
  mMutex.lock();
  QgsPostgresConnPoolGroups::iterator it = mGroups.find( conn->connInfo() );
  Q_ASSERT( it != mGroups.end() );
  QgsPostgresConnPoolGroup* group = *it;
  mMutex.unlock();

  group->release( conn );
}
