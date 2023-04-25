/***************************************************************************
   qgsredshiftconnpool.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTCONNPOOL_H
#define QGSREDSHIFTCONNPOOL_H

#include "qgsconnectionpool.h"
#include "qgsredshiftconn.h"

inline QString qgsConnectionPool_ConnectionToName( QgsRedshiftConn *c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( const QString &connInfo, QgsRedshiftConn *&c )
{
  c = QgsRedshiftConn::connectDb( connInfo, true, false );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsRedshiftConn *c )
{
  c->unref(); // will delete itself
}

inline void qgsConnectionPool_InvalidateConnection( QgsRedshiftConn *c )
{
  Q_UNUSED( c )
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsRedshiftConn *c )
{
  Q_UNUSED( c )
  return true;
}

class QgsRedshiftConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsRedshiftConn *>
{
    Q_OBJECT

  public:
    explicit QgsRedshiftConnPoolGroup( const QString &name ) : QgsConnectionPoolGroup<QgsRedshiftConn *>( name )
    {
      initTimer( this );
    }

  protected slots:
    void handleConnectionExpired()
    {
      onConnectionExpired();
    }
    void startExpirationTimer()
    {
      expirationTimer->start();
    }
    void stopExpirationTimer()
    {
      expirationTimer->stop();
    }

  protected:
    Q_DISABLE_COPY( QgsRedshiftConnPoolGroup )
};

//! Redshift connection pool - singleton
class QgsRedshiftConnPool : public QgsConnectionPool<QgsRedshiftConn *, QgsRedshiftConnPoolGroup>
{
  public:
    static QgsRedshiftConnPool *instance();

    static void cleanupInstance();

  protected:
    Q_DISABLE_COPY( QgsRedshiftConnPool )

  private:
    QgsRedshiftConnPool();
    ~QgsRedshiftConnPool() override;

    static QgsRedshiftConnPool *sInstance;
};

#endif // QGSREDSHIFTCONNPOOL_H
