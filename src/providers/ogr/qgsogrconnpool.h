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
  c->ds = OGROpen( connInfo.toUtf8().constData(), false, nullptr );
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
    explicit QgsOgrConnPoolGroup( QString name )
        : QgsConnectionPoolGroup<QgsOgrConn*>( name )
    { initTimer( this ); }

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

    // NOTE: first call to this function initializes the
    //       singleton.
    // WARNING: concurrent call from multiple threads may result
    //          in multiple instances being created, and memory
    //          leaking at exit.
    //
    static QgsOgrConnPool* instance();

    // Singleton cleanup
    //
    // Make sure nobody is using the instance before calling
    // this function.
    //
    // WARNING: concurrent call from multiple threads may result
    //          in double-free of the instance.
    //
    static void cleanupInstance();

  protected:
    Q_DISABLE_COPY( QgsOgrConnPool )

  private:
    QgsOgrConnPool();
    ~QgsOgrConnPool();
    static QgsOgrConnPool *mInstance;
};


#endif // QGSOGRCONNPOOL_H
