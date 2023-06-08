/***************************************************************************
    qgssqlconnpool.h
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

#ifndef QGSSQLCONNPOOL_H
#define QGSSQLCONNPOOL_H
#define SIP_NO_FILE

#include "qgsconnectionpool.h"
#include "qgslogger.h"

/**
 * \ingroup core
 * \brief Template class for SQL based provider connection pool group.
 * \note not available in Python bindings.
 * \since QGIS 3.32
 */
template<typename T>
class QgsSqlConnectionPoolGroup : public QgsConnectionPoolGroup<T *>
{
  public:

    /**
     * The descendants inherit from QObject and pass a self reference.
     */
    explicit QgsSqlConnectionPoolGroup( const QString &name, QObject *qobject ) :
      QgsConnectionPoolGroup<T*>( name )
    {
      QgsConnectionPoolGroup<T *>::initTimer( qobject );
    }

    using typename QgsConnectionPoolGroup<T *>::Item;
    using QgsConnectionPoolGroup<T *>::mConnections;
    ~QgsSqlConnectionPoolGroup() override
    {
      for ( const Item &item : std::as_const( mConnections ) )
      {
        connectionDestroy( item.connection );
      }
    }
    void connectionCreate( const QString &connectionInfo, T *&connection ) override
    {
      connection = T::connectDb( connectionInfo, true, false );
    }

    void connectionDestroy( T *connection ) override
    {
      connection->unref(); // will delete itself
    }

    void invalidateConnection( T * ) override {}

    bool connectionIsValid( T * ) override
    {
      return true;
    }
};

/**
 * \ingroup core
 * \brief Template class for SQL based provider connection pool.
 * \note not available in Python bindings.
 * \since QGIS 3.32
 */
template<typename T, typename T_Group>
class QgsSqlConnectionPool : public QgsConnectionPool<T *, T_Group>
{
  public:

    /**
     * \returns the instance singleton \a sInstance.
     */
    static QgsSqlConnectionPool<T, T_Group> *instance()
    {
      if ( !sInstance )
        sInstance = new QgsSqlConnectionPool<T, T_Group>();
      return sInstance;
    }

    /**
     * Reset the global instance of the connection pool.
     */
    static void cleanupInstance()
    {
      delete sInstance;
      sInstance = nullptr;
    }


    /**
     * Call instance and acquire a connection, return a shared pointer with a destructor
     * that will call the connection pool instance's release of the connection.
     */
    static std::shared_ptr<T> getConnectionFromInstance( const QString &connInfo )
    {
      return std::shared_ptr<T>( instance()->acquireConnection( connInfo ),
                                 []( T * connection )
      {
        if ( connection )
          instance()->releaseConnection( connection );
      } );
    }

    QString connectionToName( T *connection ) override
    {
      return connection->connInfo();
    }

  protected:

    /**
     * Constructor/Destructor implementation for the sole purpose of debugging.
     */
    QgsSqlConnectionPool<T, T_Group>() : QgsConnectionPool<T *, T_Group>()
    {
      QgsDebugCall;
    }

    ~QgsSqlConnectionPool<T, T_Group>() override
    {
      QgsDebugCall;
    }
  private:
    static inline QgsSqlConnectionPool<T, T_Group> *sInstance = nullptr;
};


#endif // QGSSQLCONNPOOL_H
