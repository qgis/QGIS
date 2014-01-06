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

#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include <QStack>
#include <QTime>
#include <QTimer>

class QgsPostgresConn;


//! stores data related to one server
class QgsPostgresConnPoolGroup : public QObject
{
  Q_OBJECT
public:

  static const int maxConcurrentConnections;

  struct Item
  {
    QgsPostgresConn* c;
    QTime lastUsedTime;
  };

  QgsPostgresConnPoolGroup( const QString& ci );
  ~QgsPostgresConnPoolGroup();

  QgsPostgresConn* acquire();

  void release( QgsPostgresConn* conn );

protected slots:
  void handleConnectionExpired();

protected:
  Q_DISABLE_COPY(QgsPostgresConnPoolGroup)

  QString connInfo;
  QStack<Item> conns;
  QMutex connMutex;
  QSemaphore sem;
  QTimer expirationTimer;
};

typedef QMap<QString, QgsPostgresConnPoolGroup*> QgsPostgresConnPoolGroups;

/**
 * Class responsible for keeping a pool of open connections to PostgreSQL servers.
 * This is desired to avoid the overhead of creation of new connection everytime.
 *
 * The class is a singleton. The methods are thread safe.
 *
 * The connection pool has a limit on maximum number of concurrent connections
 * (per server), once the limit is reached, the acquireConnection() function
 * will block. All connections that have been acquired must be then released
 * with releaseConnection() function.
 *
 * When the connections are not used for some time, they will get closed automatically
 * to save resources.
 *
 * \todo Make the connection pool available also for read-write connections.
 *
 */
class QgsPostgresConnPool
{
public:

  static QgsPostgresConnPool* instance();

  //! Try to acquire a connection: if no connections are available, the thread will get blocked.
  //! @return initialized connection or null on error
  QgsPostgresConn* acquireConnection( const QString& connInfo );

  //! Release an existing connection so it will get back into the pool and can be reused
  void releaseConnection( QgsPostgresConn* conn );

protected:
  QgsPostgresConnPoolGroups mGroups;
  QMutex mMutex;

  static QgsPostgresConnPool* mInstance;
};

#endif // QGSPOSTGRESCONNPOOL_H
