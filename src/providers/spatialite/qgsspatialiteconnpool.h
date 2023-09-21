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

class QgsSpatiaLiteConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsSqliteHandle *>
{
    Q_OBJECT

  public:
    explicit QgsSpatiaLiteConnPoolGroup( const QString &name ) : QgsConnectionPoolGroup<QgsSqliteHandle*>( name ) { initTimer( this ); }
    ~QgsSpatiaLiteConnPoolGroup() override
    {
      for ( Item &item : mConnections )
      {
        QgsSqliteHandle::closeDb( item.connection );  // will delete itself
      }
    }
    void connectionCreate( const QString &connectionInfo, QgsSqliteHandle *&connection ) override;
    void connectionDestroy( QgsSqliteHandle *connection ) override;
    void invalidateConnection( QgsSqliteHandle *connection ) override;
    bool connectionIsValid( QgsSqliteHandle *connection ) override;

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { mExpirationTimer->start(); }
    void stopExpirationTimer() { mExpirationTimer->stop(); }

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
    QString connectionToName( QgsSqliteHandle *connection ) override;
};


#endif // QGSSPATIALITECONPOOL_H
