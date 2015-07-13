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
    QgsOgrConnPoolGroup( QString name ) : QgsConnectionPoolGroup<QgsOgrConn*>( name ), mRefCount( 0 ) { initTimer( this ); }
    void ref() { ++mRefCount; }
    bool unref()
    {
      Q_ASSERT( mRefCount > 0 );
      return --mRefCount == 0;
    }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsOgrConnPoolGroup )

  private:
    int mRefCount;

};

/** Ogr connection pool - singleton */
class QgsOgrConnPool : public QgsConnectionPool<QgsOgrConn*, QgsOgrConnPoolGroup>
{
  public:
    static QgsOgrConnPool* instance();

    void ref( const QString& connInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connInfo );
      if ( it == mGroups.end() )
        it = mGroups.insert( connInfo, new QgsOgrConnPoolGroup( connInfo ) );
      it.value()->ref();
      mMutex.unlock();
    }

    void unref( const QString& connInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connInfo );
      Q_ASSERT( it != mGroups.end() );
      if ( it.value()->unref() )
      {
        delete it.value();
        mGroups.erase( it );
      }
      mMutex.unlock();
    }

  protected:
    Q_DISABLE_COPY( QgsOgrConnPool )

  private:
    QgsOgrConnPool();
    ~QgsOgrConnPool();

    static QgsOgrConnPool sInstance;
};


#endif // QGSOGRCONNPOOL_H
