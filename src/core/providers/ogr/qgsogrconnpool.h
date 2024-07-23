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
#include "qgsogrprovidermetadata.h"
#include "qgsogrproviderutils.h"
#include <gdal.h>
#include "qgis_sip.h"
#include <cpl_string.h>

///@cond PRIVATE
#define SIP_NO_FILE

struct QgsOgrConn
{
  QString path;
  GDALDatasetH ds;
  bool valid;
};

inline QString qgsConnectionPool_ConnectionToName( QgsOgrConn *c )
{
  return c->path;
}

inline void qgsConnectionPool_ConnectionCreate( const QString &connInfo, QgsOgrConn *&c )
{
  c = new QgsOgrConn;

  const QVariantMap parts = QgsOgrProviderMetadata().decodeUri( connInfo );
  const QString fullPath = parts.value( QStringLiteral( "vsiPrefix" ) ).toString()
                           + parts.value( QStringLiteral( "path" ) ).toString()
                           + parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
  const QStringList openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();
  char **papszOpenOptions = nullptr;
  for ( const QString &option : openOptions )
  {
    papszOpenOptions = CSLAddString( papszOpenOptions,
                                     option.toUtf8().constData() );
  }
  c->ds = QgsOgrProviderUtils::GDALOpenWrapper( fullPath.toUtf8().constData(), false, papszOpenOptions, nullptr );
  CSLDestroy( papszOpenOptions );
  c->path = connInfo;
  c->valid = true;
}

inline void qgsConnectionPool_ConnectionDestroy( QgsOgrConn *c )
{
  QgsOgrProviderUtils::GDALCloseWrapper( c->ds );
  delete c;
}

inline void qgsConnectionPool_InvalidateConnection( QgsOgrConn *c )
{
  c->valid = false;
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsOgrConn *c )
{
  return c->valid;
}

class QgsOgrConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsOgrConn *>
{
    Q_OBJECT

  public:
    explicit QgsOgrConnPoolGroup( const QString &name )
      : QgsConnectionPoolGroup<QgsOgrConn*>( name )
    {
      initTimer( this );
    }

    QgsOgrConnPoolGroup( const QgsOgrConnPoolGroup &other ) = delete;
    QgsOgrConnPoolGroup &operator=( const QgsOgrConnPoolGroup &other ) = delete;

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

  private:
    int mRefCount = 0;

};

//! Ogr connection pool - singleton
class QgsOgrConnPool : public QgsConnectionPool<QgsOgrConn *, QgsOgrConnPoolGroup>
{
  public:

    // NOTE: first call to this function initializes the
    //       singleton.
    // WARNING: concurrent call from multiple threads may result
    //          in multiple instances being created, and memory
    //          leaking at exit.
    //
    static QgsOgrConnPool *instance();

    // Singleton cleanup
    //
    // Make sure nobody is using the instance before calling
    // this function.
    //
    // WARNING: concurrent call from multiple threads may result
    //          in double-free of the instance.
    //
    static void cleanupInstance();

    QgsOgrConnPool( const QgsOgrConnPool &other ) = delete;
    QgsOgrConnPool &operator=( const QgsOgrConnPool &other ) = delete;

    /**
     * \brief Increases the reference count on the connection pool for the specified connection.
     * \param connInfo The connection string.
     * \note
     *     Any user of the connection pool needs to increase the reference count
     *     before it acquires any connections and decrease the reference count after
     *     releasing all acquired connections to ensure that all open OGR handles
     *     are freed when and only when no one is using the pool anymore.
     */
    void ref( const QString &connInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connInfo );
      if ( it == mGroups.end() )
        it = mGroups.insert( connInfo, new QgsOgrConnPoolGroup( connInfo ) );
      it.value()->ref();
      mMutex.unlock();
    }

    /**
     * \brief Decrease the reference count on the connection pool for the specified connection.
     * \param connInfo The connection string.
     */
    void unref( const QString &connInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connInfo );
      if ( it == mGroups.end() )
      {
        mMutex.unlock();
        return;
      }

      if ( it.value()->unref() )
      {
        it.value()->deleteLater();
        mGroups.erase( it );
      }
      mMutex.unlock();
    }

  private:
    QgsOgrConnPool();
    ~QgsOgrConnPool() override;
    static QgsOgrConnPool *sInstance;
};

///@endcond
#endif // QGSOGRCONNPOOL_H
