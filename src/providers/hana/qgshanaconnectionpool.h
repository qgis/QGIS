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

#include <memory>
#include <QMutex>

inline QString qgsConnectionPool_ConnectionToName( QgsHanaConnection *c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( const QgsDataSourceUri &uri, QgsHanaConnection *&c )
{
  c = QgsHanaConnection::createConnection( uri );
}

inline void qgsConnectionPool_ConnectionDestroy( QgsHanaConnection *c )
{
  delete c;
}

inline void qgsConnectionPool_InvalidateConnection( QgsHanaConnection *c )
{
  Q_UNUSED( c );
}

inline bool qgsConnectionPool_ConnectionIsValid( QgsHanaConnection *c )
{
  Q_UNUSED( c );
  return true;
}

class QgsHanaConnectionPoolGroup
  : public QObject,
    public QgsConnectionPoolGroup<QgsHanaConnection *>
{
    Q_OBJECT

  public:
    explicit QgsHanaConnectionPoolGroup( const QString &name );

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY( QgsHanaConnectionPoolGroup )
};

class QgsHanaConnectionPool
  : public QgsConnectionPool<QgsHanaConnection *, QgsHanaConnectionPoolGroup>
{
  public:
    static QgsHanaConnection *getConnection( const QString &connInfo );
    static void returnConnection( QgsHanaConnection *conn );
    static void cleanupInstance();

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
