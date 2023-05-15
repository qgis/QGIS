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

class QgsOgrConnPoolGroup : public QObject, public QgsConnectionPoolGroup<QgsOgrConn *>
{
    Q_OBJECT

  public:
    explicit QgsOgrConnPoolGroup( const QString &name )
      : QgsConnectionPoolGroup<QgsOgrConn*>( name )
    {
      initTimer( this );
    }

    //! QgsOgrConnPoolGroup cannot be copied
    QgsOgrConnPoolGroup( const QgsOgrConnPoolGroup &other ) = delete;

    //! QgsOgrConnPoolGroup cannot be copied
    QgsOgrConnPoolGroup &operator=( const QgsOgrConnPoolGroup &other ) = delete;

    ~QgsOgrConnPoolGroup() override
    {
      for ( const Item &item : std::as_const( mConnections ) )
      {
        destroyOgrConn( item.connection );
      }
    }

    void connectionCreate( const QString &connectionInfo, QgsOgrConn *&connection ) override;
    void connectionDestroy( QgsOgrConn *connection ) override;
    void invalidateConnection( QgsOgrConn *connection ) override;
    bool connectionIsValid( QgsOgrConn *connection ) override;
    void ref() { ++mRefCount; }
    bool unref()
    {
      Q_ASSERT( mRefCount > 0 );
      return --mRefCount == 0;
    }

    inline void destroyOgrConn( QgsOgrConn *connection )
    {
      QgsOgrProviderUtils::GDALCloseWrapper( connection->ds );
      delete connection;
    }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { mExpirationTimer->start(); }
    void stopExpirationTimer() { mExpirationTimer->stop(); }

  private:
    int mRefCount = 0;

};

//! Ogr connection pool - singleton
class QgsOgrConnPool : public QgsConnectionPool<QgsOgrConn *, QgsOgrConnPoolGroup>
{
  public:
    QString connectionToName( QgsOgrConn *connection ) override;
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

    //! QgsOgrConnPool cannot be copied
    QgsOgrConnPool( const QgsOgrConnPool &other ) = delete;

    //! QgsOgrConnPool cannot be copied
    QgsOgrConnPool &operator=( const QgsOgrConnPool &other ) = delete;

    /**
     * \brief Increases the reference count on the connection pool for the specified connection.
     * \param connectionInfo The connection string.
     * \note
     *     Any user of the connection pool needs to increase the reference count
     *     before it acquires any connections and decrease the reference count after
     *     releasing all acquired connections to ensure that all open OGR handles
     *     are freed when and only when no one is using the pool anymore.
     */
    void ref( const QString &connectionInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connectionInfo );
      if ( it == mGroups.end() )
        it = mGroups.insert( connectionInfo, new QgsOgrConnPoolGroup( connectionInfo ) );
      it.value()->ref();
      mMutex.unlock();
    }

    /**
     * \brief Decrease the reference count on the connection pool for the specified connection.
     * \param connectionInfo The connection string.
     */
    void unref( const QString &connectionInfo )
    {
      mMutex.lock();
      T_Groups::iterator it = mGroups.find( connectionInfo );
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
