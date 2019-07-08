/***************************************************************************
   qgshanaconnectionpool.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

inline QString qgsConnectionPool_ConnectionToName( QgsHanaConnection *c )
{
  return c->connInfo();
}

inline void qgsConnectionPool_ConnectionCreate( const QgsDataSourceUri &uri, QgsHanaConnection *&c )
{
  c = QgsHanaConnection::createConnection( uri );
}

inline void qgsConnectionPool_ConnectionDestroy(QgsHanaConnection *c )
{
  c->disconnect();
}

inline void qgsConnectionPool_InvalidateConnection(QgsHanaConnection *c )
{
  Q_UNUSED( c );
}

inline bool qgsConnectionPool_ConnectionIsValid(QgsHanaConnection *c )
{
  Q_UNUSED( c );
  return true;
}

class QgsHanaConnectionPoolGroup
  : public QObject, public QgsConnectionPoolGroup<QgsHanaConnection*>
{
    Q_OBJECT

  public:
    explicit QgsHanaConnectionPoolGroup(const QString &name);

  protected slots:
    void handleConnectionExpired() { onConnectionExpired(); }
    void startExpirationTimer() { expirationTimer->start(); }
    void stopExpirationTimer() { expirationTimer->stop(); }

  protected:
    Q_DISABLE_COPY(QgsHanaConnectionPoolGroup)
};

class QgsHanaConnectionPool
  : public QgsConnectionPool<QgsHanaConnection*, QgsHanaConnectionPoolGroup>
{
  public:
    static QgsHanaConnectionPool *instance();
    static void cleanupInstance();

  protected:
    Q_DISABLE_COPY(QgsHanaConnectionPool)

  private:
    QgsHanaConnectionPool();
    ~QgsHanaConnectionPool() override;

    static QgsHanaConnectionPool *sInstance;
};

#endif // QGSHANACONNECTIONPOOL_H
