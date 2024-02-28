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

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"

#include <QCoreApplication>
#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include <QStack>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>

#define CONN_POOL_EXPIRATION_TIME           60    // in seconds
#define CONN_POOL_SPARE_CONNECTIONS          2    // number of spare connections in case all the base connections are used but we have a nested request with the risk of a deadlock


/**
 * \ingroup core
 * \brief Template that stores data related to a connection to a single server or datasource.
 *
 * It is assumed that following functions exist:
 *
 * - void qgsConnectionPool_ConnectionCreate(QString name, T& c)  ... create a new connection
 * - void qgsConnectionPool_ConnectionDestroy(T c)                ... destroy the connection
 * - QString qgsConnectionPool_ConnectionToName(T c)              ... lookup connection's name (path)
 * - void qgsConnectionPool_InvalidateConnection(T c)             ... flag a connection as invalid
 * - bool qgsConnectionPool_ConnectionIsValid(T c)                ... return whether a connection is valid
 *
 * Because of issues with templates and QObject's signals and slots, this class only provides helper functions for QObject-related
 * functionality - the place which uses the template is resonsible for:
 *
 * - being derived from QObject
 * - calling initTimer( this ) in constructor
 * - having handleConnectionExpired() slot that calls onConnectionExpired()
 * - having startExpirationTimer(), stopExpirationTimer() slots to start/stop the expiration timer
 *
 * For an example on how to use the template class, have a look at the implementation in Postgres/SpatiaLite providers.
 * \note not available in Python bindings
 */
template <typename T>
class QgsConnectionPoolGroup
{
  public:

    struct Item
    {
      T c;
      QTime lastUsedTime;
    };

    QgsConnectionPoolGroup( const QString &ci )
      : connInfo( ci )
      , sem( QgsApplication::instance()->maxConcurrentConnectionsPerPool() + CONN_POOL_SPARE_CONNECTIONS )
    {
    }

    ~QgsConnectionPoolGroup()
    {
      for ( const Item &item : std::as_const( conns ) )
      {
        qgsConnectionPool_ConnectionDestroy( item.c );
      }
    }

    //! QgsConnectionPoolGroup cannot be copied
    QgsConnectionPoolGroup( const QgsConnectionPoolGroup &other ) = delete;
    //! QgsConnectionPoolGroup cannot be copied
    QgsConnectionPoolGroup &operator=( const QgsConnectionPoolGroup &other ) = delete;

    /**
     * Try to acquire a connection for a maximum of \a timeout milliseconds.
     * If \a timeout is a negative value the calling thread will be blocked
     * until a connection becomes available. This is the default behavior.
     *
     * \returns initialized connection or NULLPTR if unsuccessful
     */
    T acquire( int timeout, bool requestMayBeNested )
    {
      const int requiredFreeConnectionCount = requestMayBeNested ? 1 : 3;
      // we are going to acquire a resource - if no resource is available, we will block here
      if ( timeout >= 0 )
      {
        if ( !sem.tryAcquire( requiredFreeConnectionCount, timeout ) )
          return nullptr;
      }
      else
      {
        // we should still be able to use tryAcquire with a negative timeout here, but
        // tryAcquire is broken on Qt > 5.8 with negative timeouts - see
        // https://bugreports.qt.io/browse/QTBUG-64413
        // https://lists.osgeo.org/pipermail/qgis-developer/2017-November/050456.html
        sem.acquire( requiredFreeConnectionCount );
      }
      sem.release( requiredFreeConnectionCount - 1 );

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
        return nullptr;
      }

      connMutex.lock();
      acquiredConns.append( c );
      connMutex.unlock();
      return c;
    }

    void release( T conn )
    {
      connMutex.lock();
      acquiredConns.removeAll( conn );
      if ( !qgsConnectionPool_ConnectionIsValid( conn ) )
      {
        qgsConnectionPool_ConnectionDestroy( conn );
      }
      else
      {
        Item i;
        i.c = conn;
        i.lastUsedTime = QTime::currentTime();
        conns.push( i );

        if ( !expirationTimer->isActive() )
        {
          // will call the slot directly or queue the call (if the object lives in a different thread)
          QMetaObject::invokeMethod( expirationTimer->parent(), "startExpirationTimer" );
        }
      }

      connMutex.unlock();

      sem.release(); // this can unlock a thread waiting in acquire()
    }

    void invalidateConnections()
    {
      connMutex.lock();
      for ( const Item &i : std::as_const( conns ) )
      {
        qgsConnectionPool_ConnectionDestroy( i.c );
      }
      conns.clear();
      for ( T c : std::as_const( acquiredConns ) )
        qgsConnectionPool_InvalidateConnection( c );
      connMutex.unlock();
    }

  protected:

    void initTimer( QObject *parent )
    {
      expirationTimer = new QTimer( parent );
      expirationTimer->setInterval( CONN_POOL_EXPIRATION_TIME * 1000 );
      QObject::connect( expirationTimer, SIGNAL( timeout() ), parent, SLOT( handleConnectionExpired() ) );

      // just to make sure the object belongs to main thread and thus will get events
      if ( qApp )
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
    QTimer *expirationTimer = nullptr;

};


/**
 * \ingroup core
 * \brief Template class responsible for keeping a pool of open connections.
 *
 * This is desired to avoid the overhead of creation of new connection every time.
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
 * \note not available in Python bindings
 */
template <typename T, typename T_Group>
class QgsConnectionPool
{
  public:

    typedef QMap<QString, T_Group *> T_Groups;

    virtual ~QgsConnectionPool()
    {
      mMutex.lock();
      for ( T_Group *group : std::as_const( mGroups ) )
      {
        delete group;
      }
      mGroups.clear();
      mMutex.unlock();
    }

    /**
     * Try to acquire a connection for a maximum of \a timeout milliseconds.
     * If \a timeout is a negative value the calling thread will be blocked
     * until a connection becomes available. This is the default behavior.
     *
     * The optional \a feedback argument can be used to cancel the request
     * before the connection is acquired.
     *
     * \returns initialized connection or NULLPTR if unsuccessful
     */
    T acquireConnection( const QString &connInfo, int timeout = -1, bool requestMayBeNested = false, QgsFeedback *feedback = nullptr )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( connInfo );
      if ( it == mGroups.end() )
      {
        it = mGroups.insert( connInfo, new T_Group( connInfo ) );
      }
      T_Group *group = *it;
      mMutex.unlock();

      if ( feedback )
      {
        QElapsedTimer timer;
        timer.start();

        while ( !feedback->isCanceled() )
        {
          if ( T conn = group->acquire( 300, requestMayBeNested ) )
            return conn;

          if ( timeout > 0 && timer.elapsed() >= timeout )
            return nullptr;
        }
        return nullptr;
      }
      else
      {
        return group->acquire( timeout, requestMayBeNested );
      }
    }

    //! Release an existing connection so it will get back into the pool and can be reused
    void releaseConnection( T conn )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( qgsConnectionPool_ConnectionToName( conn ) );
      Q_ASSERT( it != mGroups.end() );
      T_Group *group = *it;
      mMutex.unlock();

      group->release( conn );
    }

    /**
     * Invalidates all connections to the specified resource.
     * The internal state of certain handles (for instance OGR) are altered
     * when a dataset is modified. Consequently, all open handles need to be
     * invalidated when such datasets are changed to ensure the handles are
     * refreshed. See the OGR provider for an example where this is needed.
     */
    void invalidateConnections( const QString &connInfo )
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
