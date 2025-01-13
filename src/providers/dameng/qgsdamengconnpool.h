/***************************************************************************
    qgsdamengconnpool.h
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGCONNPOOL_H
#define QGSDAMENGCONNPOOL_H

#include "qgsconnectionpool.h"
#include "qgsdamengconn.h"


inline QString qgsConnectionPool_ConnectionToName( QgsDamengConn *c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( const QString &connInfo, QgsDamengConn *&c )
{
  c = QgsDamengConn::connectDb( connInfo, true, false );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsDamengConn *c )
{
  c->unref(); // will delete itself
}

inline void qgsConnectionPool_InvalidateConnection( QgsDamengConn *c )
{
  Q_UNUSED( c )
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsDamengConn *c )
{
  Q_UNUSED( c )
  return c->dmConnection()->dmDriver->isConnect();
}


class QgsDamengConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsDamengConn *>
{
    Q_OBJECT

  public:
    explicit QgsDamengConnPoolGroup( const QString &name )
      : QgsConnectionPoolGroup<QgsDamengConn*>( name ) { initTimer( this ); }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsDamengConnPoolGroup )

};

//! Dameng connection pool - singleton
class QgsDamengConnPool : public QgsConnectionPool< QgsDamengConn *, QgsDamengConnPoolGroup >
{
  public:
    static QgsDamengConnPool *instance();

    static void cleanupInstance();

  protected:
    Q_DISABLE_COPY( QgsDamengConnPool )

  private:
    QgsDamengConnPool();
    ~QgsDamengConnPool() override;

    static QgsDamengConnPool *sInstance;
};


#endif // QGSDAMENGCONNPOOL_H
