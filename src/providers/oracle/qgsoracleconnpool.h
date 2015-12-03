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


inline QString qgsConnectionPool_ConnectionToName( QgsOracleConn* c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( QgsDataSourceURI uri, QgsOracleConn*& c )
{
  c = QgsOracleConn::connectDb( uri );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsOracleConn* c )
{
  c->disconnect(); // will delete itself
}

inline void qgsConnectionPool_InvalidateConnection( QgsOracleConn* c )
{
  Q_UNUSED( c );
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsOracleConn* c )
{
  Q_UNUSED( c );
  return true;
}


class QgsOracleConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsOracleConn*>
{
    Q_OBJECT

  public:
    explicit QgsOracleConnPoolGroup( QString name ) : QgsConnectionPoolGroup<QgsOracleConn*>( name ) { initTimer( this ); }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsOracleConnPoolGroup )

};

/** Oracle connection pool - singleton */
class QgsOracleConnPool : public QgsConnectionPool<QgsOracleConn*, QgsOracleConnPoolGroup>
{
  public:
    static QgsOracleConnPool* instance();

  protected:
    Q_DISABLE_COPY( QgsOracleConnPool )

  private:
    QgsOracleConnPool();
    ~QgsOracleConnPool();

    static QgsOracleConnPool sInstance;
};


#endif // QGSORACLECONNPOOL_H
