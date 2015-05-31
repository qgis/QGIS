/***************************************************************************
    qgsogrconnpool.h
    ---------------------
    begin                : May 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRCONNPOOL_H
#define QGSOGRCONNPOOL_H

#include "qgsconnectionpool.h"
#include <ogr_api.h>


struct QgsOgrConn
{
  QString path;
  OGRDataSourceH ds;
  bool valid;
};

inline QString qgsConnectionPool_ConnectionToName( QgsOgrConn* c )
{
  return c->path;
}

inline void qgsConnectionPool_ConnectionCreate( QString connInfo, QgsOgrConn*& c )
{
  c = new QgsOgrConn;
  c->ds = OGROpen( connInfo.toUtf8().constData(), false, NULL );
  c->path = connInfo;
  c->valid = true;
}

inline void qgsConnectionPool_ConnectionDestroy( QgsOgrConn* c )
{
  OGR_DS_Destroy( c->ds );
  delete c;
}

inline void qgsConnectionPool_InvalidateConnection( QgsOgrConn* c )
{
  c->valid = false;
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsOgrConn* c )
{
  return c->valid;
}

class QgsOgrConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsOgrConn*>
{
    Q_OBJECT

  public:
    QgsOgrConnPoolGroup( QString name ) : QgsConnectionPoolGroup<QgsOgrConn*>( name ) { initTimer( this ); }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsOgrConnPoolGroup )

};

/** Ogr connection pool - singleton */
class QgsOgrConnPool : public QgsConnectionPool<QgsOgrConn*, QgsOgrConnPoolGroup>
{
  public:
    static QgsOgrConnPool* instance();

  protected:
    Q_DISABLE_COPY( QgsOgrConnPool )

  private:
    QgsOgrConnPool();
    ~QgsOgrConnPool();

    static QgsOgrConnPool sInstance;
};


#endif // QGSOGRCONNPOOL_H
