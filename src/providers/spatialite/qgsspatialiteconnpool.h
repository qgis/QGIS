/***************************************************************************
    qgsspatialiteconnpool.h
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

#ifndef QGSSPATIALITECONPOOL_H
#define QGSSPATIALITECONPOOL_H

#include "qgsconnectionpool.h"
#include "qgsspatialiteconnection.h"

inline QString qgsConnectionPool_ConnectionToName( QgsSqliteHandle *c )
{
  return c->dbPath();
}

inline void qgsConnectionPool_ConnectionCreate( const QString &connInfo, QgsSqliteHandle *&c )
{
  c = QgsSqliteHandle::openDb( connInfo, false );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsSqliteHandle *c )
{
  QgsSqliteHandle::closeDb( c ); // will delete itself
}

inline void qgsConnectionPool_InvalidateConnection( QgsSqliteHandle *c )
{
  /* Invalidation is used in particular by the WFS provider that uses a */
  /* temporary SpatiaLite DB and want to delete it at some point. For that */
  /* it must invalidate all handles pointing to it */
  c->invalidate();
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsSqliteHandle *c )
{
  return c->isValid();
}


class QgsSpatiaLiteConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsSqliteHandle *>
{
    Q_OBJECT

  public:
    explicit QgsSpatiaLiteConnPoolGroup( const QString &name )
      : QgsConnectionPoolGroup<QgsSqliteHandle *>( name ) { initTimer( this ); }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsSpatiaLiteConnPoolGroup )
};

//! SpatiaLite connection pool - singleton
class QgsSpatiaLiteConnPool : public QgsConnectionPool<QgsSqliteHandle *, QgsSpatiaLiteConnPoolGroup>
{
    static QgsSpatiaLiteConnPool *sInstance;

  public:
    static QgsSpatiaLiteConnPool *instance();

    // Singleton cleanup
    //
    // Make sure nobody is using the instance before calling
    // this function.
    //
    // WARNING: concurrent call from multiple threads may result
    //          in double-free of the instance.
    //
    static void cleanupInstance();
};


#endif // QGSSPATIALITECONPOOL_H
