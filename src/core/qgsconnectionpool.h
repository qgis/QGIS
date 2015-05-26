/***************************************************************************
    qgsconnectionpool.h
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

#ifndef QGSCONNECTIONPOOL_H
#define QGSCONNECTIONPOOL_H

#include <QCoreApplication>
#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include <QStack>
#include <QTime>
#include <QTimer>
#include <QThread>

#include "qgslogger.h"

#define CONN_POOL_MAX_CONCURRENT_CONNS      4
#define CONN_POOL_EXPIRATION_TIME           60    // in seconds


/*! Template that stores data related to one server.
 *
 * It is assumed that following functions exist:
 * - void qgsConnectionPool_ConnectionCreate(QString name, T& c)  ... create a new connection
 * - void qgsConnectionPool_ConnectionDestroy(T c)                ... destroy the connection
 * - QString qgsConnectionPool_ConnectionToName(T c)              ... lookup connection's name (path)
 * - void qgsConnectionPool_InvalidateConnection(T c)             ... flag a connection as invalid
 * - bool qgsConnectionPool_ConnectionIsValid(T c)                ... return whether a connection is valid
 *
 * Because of issues with templates and QObject's signals and slots, this class only provides helper functions for QObject-related
 * functionality - the place which uses the template is resonsible for:
 * - being derived from QObject
 * - calling initTimer( this ) in constructor
 * - having handleConnectionExpired() slot that calls onConnectionExpired()
 * - having startExpirationTimer(), stopExpirationTimer() slots to start/stop the expiration timer
 *
 * For an example on how to use the template class, have a look at the implementation in postgres/spatialite providers.
 */
template <typename T>
class QgsConnectionPoolGroup
{
  public:

    static const int maxConcurrentConnections;

    struct Item
    {
      T c;
      QTime lastUsedTime;
    };

    QgsConnectionPoolGroup( const QString& ci )
        : connInfo( ci )
        , sem( CONN_POOL_MAX_CONCURRENT_CONNS )
        , expirationTimer( 0 )
    {
    }

    ~QgsConnectionPoolGroup()
    {
      foreach ( Item item, conns )
      {
        qgsConnectionPool_ConnectionDestroy( item.c );
      }
    }

    T acquire()
    {
      // we are going to acquire a resource - if no resource is available, we will block here
      sem.acquire();

      // quick (preferred) way - use cached connection
      {
        QMutexLocker locker( &connMutex );

        if ( !conns.isEmpty() )
        {
          Item i = conns.pop();
          if ( !qgsConnectionPool_ConnectionIsValid( i.c ) )
          {
            qgsConnectionPool_ConnectionDestroy( i.c );
            qgsConnectionPool_ConnectionCreate( connInfo, i.c );
          }

          // no need to run if nothing can expire
          if ( conns.isEmpty() )
          {
            // will call the slot directly or queue the call (if the object lives in a different thread)
            QMetaObject::invokeMethod( expirationTimer->parent(), "stopExpirationTimer" );
          }

          acquiredConns.append( i.c );

          return i.c;
        }
      }

      T c;
      qgsConnectionPool_ConnectionCreate( connInfo, c );
      if ( !c )
      {
        // we didn't get connection for some reason, so release the lock
        sem.release();
        return 0;
      }

      acquiredConns.append( c );
      return c;
    }

    void release( T conn )
    {
      connMutex.lock();
      acquiredConns.removeAll( conn );
      Item i;
      i.c = conn;
      i.lastUsedTime = QTime::currentTime();
      conns.push( i );

      if ( !expirationTimer->isActive() )
      {
        // will call the slot directly or queue the call (if the object lives in a different thread)
        QMetaObject::invokeMethod( expirationTimer->parent(), "startExpirationTimer" );
      }

      connMutex.unlock();

      sem.release(); // this can unlock a thread waiting in acquire()
    }

    void invalidateConnections()
    {
      connMutex.lock();
      foreach ( Item i, conns )
      {
        qgsConnectionPool_InvalidateConnection( i.c );
      }
      foreach ( T c, acquiredConns )
        qgsConnectionPool_InvalidateConnection( c );
      connMutex.unlock();
    }

  protected:

    void initTimer( QObject* parent )
    {
      expirationTimer = new QTimer( parent );
      expirationTimer->setInterval( CONN_POOL_EXPIRATION_TIME * 1000 );
      QObject::connect( expirationTimer, SIGNAL( timeout() ), parent, SLOT( handleConnectionExpired() ) );

      // just to make sure the object belongs to main thread and thus will get events
      parent->moveToThread( qApp->thread() );
    }

    void onConnectionExpired()
    {
      connMutex.lock();

      QTime now = QTime::currentTime();

      // what connections have expired?
      QList<int> toDelete;
      for ( int i = 0; i < conns.count(); ++i )
      {
        if ( conns.at( i ).lastUsedTime.secsTo( now ) >= CONN_POOL_EXPIRATION_TIME )
          toDelete.append( i );
      }

      // delete expired connections
      for ( int j = toDelete.count() - 1; j >= 0; --j )
      {
        int index = toDelete[j];
        qgsConnectionPool_ConnectionDestroy( conns[index].c );
        conns.remove( index );
      }

      if ( conns.isEmpty() )
        expirationTimer->stop();

      connMutex.unlock();
    }

  protected:

    QString connInfo;
    QStack<Item> conns;
    QList<T> acquiredConns;
    QMutex connMutex;
    QSemaphore sem;
    QTimer* expirationTimer;
};


/**
 * Template class responsible for keeping a pool of open connections.
 * This is desired to avoid the overhead of creation of new connection everytime.
 *
 * The methods are thread safe.
 *
 * The connection pool has a limit on maximum number of concurrent connections
 * (per server), once the limit is reached, the acquireConnection() function
 * will block. All connections that have been acquired must be then released
 * with releaseConnection() function.
 *
 * When the connections are not used for some time, they will get closed automatically
 * to save resources.
 *
 */
template <typename T, typename T_Group>
class QgsConnectionPool
{
  public:

    typedef QMap<QString, T_Group*> T_Groups;

    //! Try to acquire a connection: if no connections are available, the thread will get blocked.
    //! @return initialized connection or null on error
    T acquireConnection( const QString& connInfo )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( connInfo );
      if ( it == mGroups.end() )
      {
        it = mGroups.insert( connInfo, new T_Group( connInfo ) );
      }
      T_Group* group = *it;
      mMutex.unlock();

      return group->acquire();
    }

    //! Release an existing connection so it will get back into the pool and can be reused
    void releaseConnection( T conn )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( qgsConnectionPool_ConnectionToName( conn ) );
      Q_ASSERT( it != mGroups.end() );
      T_Group* group = *it;
      mMutex.unlock();

      group->release( conn );
    }

    //! Invalidates all connections to the specified resource.
    //! The internal state of certain handles (for instance OGR) are altered
    //! when a dataset is modified. Consquently, all open handles need to be
    //! invalidated when such datasets are changed to ensure the handles are
    //! refreshed. See the OGR provider for an example where this is needed.
    void invalidateConnections( const QString& connInfo )
    {
      mMutex.lock();
      if ( mGroups.contains( connInfo ) )
        mGroups[connInfo]->invalidateConnections();
      mMutex.unlock();
    }


  protected:
    T_Groups mGroups;
    QMutex mMutex;
};


#endif // QGSCONNECTIONPOOL_H
