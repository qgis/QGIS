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
      T connection;
      QTime lastUsedTime;
    };

    QgsConnectionPoolGroup( const QString &ci )
      : mConnectionInfo( ci )
      , mSemaphore( QgsApplication::instance()->maxConcurrentConnectionsPerPool() + CONN_POOL_SPARE_CONNECTIONS )
    {
    }

    virtual ~QgsConnectionPoolGroup() = 0;

    /**
     * Classes that implement this template must implement their own connection functions.
     * Create a \a connection based on connection information \a connectionInfo.
     */
    virtual void connectionCreate( const QString &connectionInfo, T &connection ) = 0;

    /**
     * Destroy the \a connection.
     */
    virtual void connectionDestroy( T connection ) = 0;

    /**
     * Invalidate the \a connection.
     */
    virtual void invalidateConnection( T connection ) = 0;

    /**
     * \returns true if the \a connection is valid.
     */
    virtual bool connectionIsValid( T connection ) = 0;

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
        if ( !mSemaphore.tryAcquire( requiredFreeConnectionCount, timeout ) )
          return nullptr;
      }
      else
      {
        // we should still be able to use tryAcquire with a negative timeout here, but
        // tryAcquire is broken on Qt > 5.8 with negative timeouts - see
        // https://bugreports.qt.io/browse/QTBUG-64413
        // https://lists.osgeo.org/pipermail/qgis-developer/2017-November/050456.html
        mSemaphore.acquire( requiredFreeConnectionCount );
      }
      mSemaphore.release( requiredFreeConnectionCount - 1 );

      // quick (preferred) way - use cached connection
      {
        QMutexLocker locker( &mConnectionMutex );

        if ( !mConnections.isEmpty() )
        {
          Item i = mConnections.pop();
          if ( !connectionIsValid( i.connection ) )
          {
            connectionDestroy( i.connection );
            connectionCreate( mConnectionInfo, i.connection );
          }


          // no need to run if nothing can expire
          if ( mConnections.isEmpty() )
          {
            // will call the slot directly or queue the call (if the object lives in a different thread)
            QMetaObject::invokeMethod( mExpirationTimer->parent(), "stopExpirationTimer" );
          }

          mAcquiredConnections.append( i.connection );

          return i.connection;
        }
      }

      T connection;
      connectionCreate( mConnectionInfo, connection );
      if ( !connection )
      {
        // we didn't get connection for some reason, so release the lock
        mSemaphore.release();
        return nullptr;
      }

      mConnectionMutex.lock();
      mAcquiredConnections.append( connection );
      mConnectionMutex.unlock();
      return connection;
    }

    /**
     * Release the \a connection if it's valid, destroy it otherwise.
     * Unlock the connection mutex and semaphore.
     */
    void release( T connection )
    {
      mConnectionMutex.lock();
      mAcquiredConnections.removeAll( connection );
      if ( !connectionIsValid( connection ) )
      {
        connectionDestroy( connection );
      }
      else
      {
        Item i;
        i.connection = connection;
        i.lastUsedTime = QTime::currentTime();
        mConnections.push( i );

        if ( !mExpirationTimer->isActive() )
        {
          // will call the slot directly or queue the call (if the object lives in a different thread)
          QMetaObject::invokeMethod( mExpirationTimer->parent(), "startExpirationTimer" );
        }
      }

      mConnectionMutex.unlock();

      mSemaphore.release(); // this can unlock a thread waiting in acquire()
    }

    void invalidateConnections()
    {
      mConnectionMutex.lock();
      for ( const Item &i : std::as_const( mConnections ) )
      {
        connectionDestroy( i.connection );
      }
      mConnections.clear();
      for ( T connection : std::as_const( mAcquiredConnections ) )
        invalidateConnection( connection );
      mConnectionMutex.unlock();
    }

  protected:

    void initTimer( QObject *parent )
    {
      mExpirationTimer = new QTimer( parent );
      mExpirationTimer->setInterval( CONN_POOL_EXPIRATION_TIME * 1000 );
      QObject::connect( mExpirationTimer, SIGNAL( timeout() ), parent, SLOT( handleConnectionExpired() ) );

      // just to make sure the object belongs to main thread and thus will get events
      if ( qApp )
        parent->moveToThread( qApp->thread() );
    }

    void onConnectionExpired()
    {
      mConnectionMutex.lock();

      QTime now = QTime::currentTime();

      // what connections have expired?
      QList<int> toDelete;
      for ( int i = 0; i < mConnections.count(); ++i )
      {
        if ( mConnections.at( i ).lastUsedTime.secsTo( now ) >= CONN_POOL_EXPIRATION_TIME )
          toDelete.append( i );
      }

      // delete expired connections
      for ( int j = toDelete.count() - 1; j >= 0; --j )
      {
        int index = toDelete[j];
        connectionDestroy( mConnections[index].connection );
        mConnections.remove( index );
      }

      if ( mConnections.isEmpty() )
        mExpirationTimer->stop();

      mConnectionMutex.unlock();
    }

  protected:

    QString mConnectionInfo;
    QStack<Item> mConnections;
    QList<T> mAcquiredConnections;
    QMutex mConnectionMutex;
    QSemaphore mSemaphore;
    QTimer *mExpirationTimer = nullptr;

};

template<typename T>
QgsConnectionPoolGroup<T>::~QgsConnectionPoolGroup() {}


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
     * \returns the name of the \a connection.
     */
    virtual QString connectionToName( T connection ) = 0;

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
    T acquireConnection( const QString &connectionInfo, int timeout = -1, bool requestMayBeNested = false, QgsFeedback *feedback = nullptr )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( connectionInfo );
      if ( it == mGroups.end() )
      {
        it = mGroups.insert( connectionInfo, new T_Group( connectionInfo ) );
      }
      T_Group *group = *it;
      mMutex.unlock();

      if ( feedback )
      {
        QElapsedTimer timer;
        timer.start();

        while ( !feedback->isCanceled() )
        {
          if ( T connection = group->acquire( 300, requestMayBeNested ) )
            return connection;

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
    void releaseConnection( T connection )
    {
      mMutex.lock();
      typename T_Groups::iterator it = mGroups.find( connectionToName( connection ) );
      Q_ASSERT( it != mGroups.end() );
      T_Group *group = *it;
      mMutex.unlock();

      group->release( connection );
    }

    /**
     * Invalidates all connections to the specified resource.
     * The internal state of certain handles (for instance OGR) are altered
     * when a dataset is modified. Consquently, all open handles need to be
     * invalidated when such datasets are changed to ensure the handles are
     * refreshed. See the OGR provider for an example where this is needed.
     */
    void invalidateConnections( const QString &connectionInfo )
    {
      mMutex.lock();
      if ( mGroups.contains( connectionInfo ) )
        mGroups[connectionInfo]->invalidateConnections();
      mMutex.unlock();
    }


  protected:
    T_Groups mGroups;
    QMutex mMutex;
};

#endif // QGSCONNECTIONPOOL_H
