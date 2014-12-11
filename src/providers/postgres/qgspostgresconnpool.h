/***************************************************************************
    qgspostgresconnpool.h
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

#ifndef QGSPOSTGRESCONNPOOL_H
#define QGSPOSTGRESCONNPOOL_H

#include "qgsconnectionpool.h"

#include "qgspostgresconn.h"


inline QString qgsConnectionPool_ConnectionToName( QgsPostgresConn* c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( QString connInfo, QgsPostgresConn*& c )
{
  c = QgsPostgresConn::connectDb( connInfo, true, false );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsPostgresConn* c )
{
  c->disconnect(); // will delete itself
}


class QgsPostgresConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsPostgresConn*>
{
    Q_OBJECT

  public:
    QgsPostgresConnPoolGroup( QString name ) : QgsConnectionPoolGroup<QgsPostgresConn*>( name ) { initTimer( this ); }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsPostgresConnPoolGroup )

};

/** PostgreSQL connection pool - singleton */
class QgsPostgresConnPool : public QgsConnectionPool<QgsPostgresConn*, QgsPostgresConnPoolGroup>
{
  public:
    static QgsPostgresConnPool* instance();

  protected:
    Q_DISABLE_COPY( QgsPostgresConnPool );

  private:
    QgsPostgresConnPool();
    ~QgsPostgresConnPool();
};


#endif // QGSPOSTGRESCONNPOOL_H
