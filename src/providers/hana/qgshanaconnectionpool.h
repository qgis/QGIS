/***************************************************************************
   qgshanaconnectionpool.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANACONNECTIONPOOL_H
#define QGSHANACONNECTIONPOOL_H

#include "qgshanaconnection.h"
#include "qgsconnectionpool.h"

#include <QtCore/qalgorithms.h>
#include <memory>
#include <QMutex>

class QgsHanaConnectionPoolGroup
  : public QObject, public QgsConnectionPoolGroup<QgsHanaConnection *>
{
    Q_OBJECT

  public:
    explicit QgsHanaConnectionPoolGroup( const QString &name )
      : QgsConnectionPoolGroup<QgsHanaConnection*>( name )
    {
      initTimer( this );
    }

    ~QgsHanaConnectionPoolGroup() override
    {
      for ( const Item &item : std::as_const( mConnections ) )
      {
        delete item.connection;
      }
    }

    void connectionCreate( const QString &uri, QgsHanaConnection *&c ) override
    {
      c = QgsHanaConnection::createConnection( uri );
    }

    void connectionDestroy( QgsHanaConnection *connection ) override
    {
      delete connection;
    }

    void invalidateConnection( QgsHanaConnection * ) override {}


    bool connectionIsValid( QgsHanaConnection * ) override
    {
      return true;
    }

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { mExpirationTimer->start(); }
    void stopExpirationTimer() { mExpirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsHanaConnectionPoolGroup )
};

class QgsHanaConnectionPool
  : public QgsConnectionPool<QgsHanaConnection *, QgsHanaConnectionPoolGroup>
{
  public:
    static QgsHanaConnection *getConnection( const QString &connectionInfo );
    static void returnConnection( QgsHanaConnection *connection );
    static void cleanupInstance();

    QString connectionToName( QgsHanaConnection *connection ) override
    {
      return connection->connInfo();
    }

  protected:
    Q_DISABLE_COPY( QgsHanaConnectionPool )

  private:
    QgsHanaConnectionPool();

  public:
    ~QgsHanaConnectionPool() override;

  private:
    static QBasicMutex sMutex;
    static std::shared_ptr<QgsHanaConnectionPool> sInstance;
};

class QgsHanaConnectionRef
{
  public:
    QgsHanaConnectionRef() = default;
    QgsHanaConnectionRef( const QString &name );
    QgsHanaConnectionRef( const QgsDataSourceUri &uri );

    QgsHanaConnectionRef( QgsHanaConnectionRef &&other ) = default;
    QgsHanaConnectionRef &operator=( QgsHanaConnectionRef && ) = delete;
    Q_DISABLE_COPY( QgsHanaConnectionRef )

    ~QgsHanaConnectionRef();

    bool isNull() const { return mConnection.get() == nullptr; }
    QgsHanaConnection &operator*() { return *mConnection; }
    QgsHanaConnection *operator->() { return mConnection.get(); }

  private:
    std::unique_ptr<QgsHanaConnection> mConnection;
};

#endif // QGSHANACONNECTIONPOOL_H
