/***************************************************************************
    qgsoracleonnpool.h
    ---------------------
    begin                : November 2015
    copyright            : (C) 2015 by Juergen Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLECONNPOOL_H
#define QGSORACLECONNPOOL_H

#include "qgsconnectionpool.h"
#include "qgsoracleconn.h"


class QgsOracleConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsOracleConn *>
{
    Q_OBJECT

  public:
    explicit QgsOracleConnPoolGroup( QString name ) : QgsConnectionPoolGroup<QgsOracleConn*>( name ) { initTimer( this ); }
    ~QgsOracleConnPoolGroup() override
    {
      for ( const Item &item : std::as_const( mConnections ) )
      {
        item.connection->disconnect(); // will delete itself
      }
    }

    void connectionCreate( const QString &connectionInfo, QgsOracleConn *&connection ) override
    {
      connection = QgsOracleConn::connectDb( connectionInfo, false );
    }

    void connectionDestroy( QgsOracleConn *connection ) override
    {
      connection->disconnect(); // will delete itself
    }

    void invalidateConnection( QgsOracleConn * ) override {};
    bool connectionIsValid( QgsOracleConn * ) override { return true;};

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { mExpirationTimer->start(); }
    void stopExpirationTimer() { mExpirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsOracleConnPoolGroup )

};

//! Oracle connection pool - singleton
class QgsOracleConnPool : public QgsConnectionPool<QgsOracleConn *, QgsOracleConnPoolGroup>
{
  public:
    static QgsOracleConnPool *instance();

    static void cleanupInstance();

    QString connectionToName( QgsOracleConn *connection ) override
    {
      return connection->connInfo();
    }

  protected:
    Q_DISABLE_COPY( QgsOracleConnPool )

  private:
    QgsOracleConnPool();
    ~QgsOracleConnPool() override;

    static QgsOracleConnPool *sInstance;
};


#endif // QGSORACLECONNPOOL_H
