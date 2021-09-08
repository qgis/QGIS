/***************************************************************************
  qgsmssqldatabase.h
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLDATABASE_H
#define QGSMSSQLDATABASE_H


#include <QMap>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <memory>

class QgsDataSourceUri;

/**
 * This is a wrapper around QSqlDatabase.
 *
 * Under normal conditions, each thread gets a dedicated connection (QSqlDatabase) for a database within a host.
 * When a transaction has been started, all threads will use a single shared QSqlDatabase and access to it
 * is controlled by a mutex.
 *
 * QtSql does not like sharing QSqlDatabase objects among threads (since Qt 5.11, QSqlDatabase::database()
 * will refuse to return a database object that was added in a different thread), but... we use proper locking
 * to avoid multiple threads using a single connection at once AND it was verified that QSqlDatabase works fine
 * in multi-threaded environment at least with MSSQL. And by the way, Oracle provider also uses a single shared
 * QSqlDatabase for transactions...
 */
class QgsMssqlDatabase
{
  public:

    /**
     * Tries to connect to a MSSQL database and returns shared pointer to the connection. On success,
     * the returned database object (QSqlDatabase) is already open and it is not necessary to call open().
     *
     * It always returns an instance, but the caller should check whether the returned instance is valid
     * before using it.
     *
     * \note The function is thread-safe
     */
    static std::shared_ptr<QgsMssqlDatabase> connectDb( const QString &uri, bool transaction = false );
    static std::shared_ptr<QgsMssqlDatabase> connectDb( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool transaction = false );

    /////

    ~QgsMssqlDatabase();

    //! Returns true if we were successful to open the database (and so we can use the database connection)
    bool isValid() const { return mDB.isOpen(); }
    //! Returns error text for the error if database failed to open
    QString errorText() const { return mDB.lastError().text(); }
    //! Returns reference to the internal database connection
    QSqlDatabase &db() { return mDB; }
    //! Returns whether this connection is used in a transaction
    bool hasTransaction() const { return mTransaction; }

  private:
    QgsMssqlDatabase( const QSqlDatabase &db, bool transaction );

    QSqlDatabase mDB;
    bool mTransaction = false;
    //! locking for transactions because with transaction enabled, one connection may be shared among multiple threads
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    std::unique_ptr<QMutex> mTransactionMutex;
#else
    std::unique_ptr<QRecursiveMutex> mTransactionMutex;
#endif

    friend class QgsMssqlQuery;

    static QString connectionName( const QString &service, const QString &host, const QString &database, bool transaction );

    /**
     * Returns a QSqlDatabase object for queries to SQL Server.
     *
     * The database may not be open -- openDatabase() should be called to
     * ensure that it is ready for use.
     */
    static QSqlDatabase getDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool transaction = false );

    static QMap<QString, std::weak_ptr<QgsMssqlDatabase> > sConnections;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static QMutex sMutex;
#else
    static QRecursiveMutex sMutex;
#endif
};


/**
 * A light wrapper around QSqlQuery that provides locking for database connections when
 * running in a transaction.
 */
class QgsMssqlQuery : public QSqlQuery
{
  public:
    explicit QgsMssqlQuery( std::shared_ptr<QgsMssqlDatabase> db )
      : QSqlQuery( db->db() )
      , mDb( db )
    {
      if ( mDb->hasTransaction() )
        mDb->mTransactionMutex->lock();
    }

    ~QgsMssqlQuery()
    {
      if ( mDb->hasTransaction() )
        mDb->mTransactionMutex->unlock();
    }

  private:
    std::shared_ptr<QgsMssqlDatabase> mDb;

};


#endif // QGSMSSQLDATABASE_H
