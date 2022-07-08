/***************************************************************************
  qgsdbquerylog.h
  ------------
  Date                 : October 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDBQUERYLOG_H
#define QGSDBQUERYLOG_H

#include "qgis_core.h"
#include "qgis.h"
#include <QString>
#include <QDateTime>

/**
 * \ingroup core
 * \class QgsDatabaseQueryLogEntry
 * \brief Encapsulates a logged database query.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsDatabaseQueryLogEntry
{
  public:

    /**
     * Constructor for QgsDatabaseQueryLogEntry.
     */
    QgsDatabaseQueryLogEntry( const QString &query = QString() );

    /**
     * Unique query ID.
     *
     * This ID will automatically be set on creation of a new QgsDatabaseQueryLogEntry object.
     */
    int queryId = 0;

    //! Database URI
    QString uri;

    //! Provider key
    QString provider;

    //! The logged database query (e.g. the SQL query)
    QString query;

    /**
     * Time when the query started (in milliseconds since epoch).
     *
     * This will be automatically recorded on creation of a new QgsDatabaseQueryLogEntry object.
     */
    quint64 startedTime = 0;

    /**
     * Time when the query finished (in milliseconds since epoch), if available.
     */
    quint64 finishedTime = 0;

    /**
     * The QGIS class which initiated the query.
     *
     * c++ code can automatically populate this through the QgsSetQueryLogClass macro.
     */
    QString initiatorClass;

    /**
     * Code file location for the query origin.
     *
     * c++ code can automatically populate this through the QgsSetQueryLogClass macro.
     */
    QString origin;

    /**
     * Number of fetched/affected rows.
     * \warning Not all providers support this information.
     */
    long long fetchedRows = -1;

    /**
     * Error reported by the provider, normally blank
     */
    QString error;

    /**
     * Canceled flag for user canceled queries.
     */
    bool canceled = false;

  private:

    static QAtomicInt sQueryId;
};

Q_DECLARE_METATYPE( QgsDatabaseQueryLogEntry );

#ifndef SIP_RUN
#include "qgsconfig.h"
constexpr int sQueryLoggerFilePrefixLength = CMAKE_SOURCE_DIR[sizeof( CMAKE_SOURCE_DIR ) - 1] == '/' ? sizeof( CMAKE_SOURCE_DIR ) + 1 : sizeof( CMAKE_SOURCE_DIR );
#define QgsSetQueryLogClass(entry, _class) entry.initiatorClass = _class; entry.origin = QString(QString( __FILE__ ).mid( sQueryLoggerFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")");
#define QGS_QUERY_LOG_ORIGIN QString(QString( __FILE__ ).mid( sQueryLoggerFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")")
#endif

/**
 * \ingroup core
 * \class QgsDatabaseQueryLog
 * \brief Handles logging of database queries.
 *
 * QgsDatabaseQueryLog is not usually directly created, but rather accessed through
 * QgsApplication::databaseQueryLog(). Generally, clients should only access the
 * static log() method to register their queries.
 *
 * ### Example
 *
 * \code{.py}
 *   # Log a database query
 *   QgsDatabaseQueryLog.log('SELECT * FROM my_table')
 * \endcode
 *
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsDatabaseQueryLog: public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new query log.
     *
     * QgsDatabaseQueryLog is not usually directly created, but rather accessed through
     * QgsApplication::databaseQueryLog().
    */
    QgsDatabaseQueryLog( QObject *parent = nullptr );

    /**
     * Enables query logging.
     *
     * If disabled, no signals will be emitted by the log. By default the log is disabled,
     * and clients must manually enable it.
     *
     * \note Not available in Python bindings
     * \see enabled()
     */
    static void setEnabled( bool enabled ) SIP_SKIP { sEnabled = enabled; }

    /**
     * Returns TRUE if logging is enabled.
     *
     * \see setEnabled()
     */
    static bool enabled() { return sEnabled; }

    /**
     * Logs a database \a query as starting.
     *
     * This method can be safely called from any thread.
     */
    static void log( const QgsDatabaseQueryLogEntry &query );

    /**
     * Records that the database \a query has finished.
     *
     * This method can be safely called from any thread.
     */
    static void finished( const QgsDatabaseQueryLogEntry &query );

  public slots:

    /**
     * Internal slot for logging queries as start.
     *
     * \note Not available in Python bindings.
     */
    void queryStartedPrivate( const QgsDatabaseQueryLogEntry &query ) SIP_SKIP;

    /**
     * Internal slot for logging queries as finished.
     *
     * \note Not available in Python bindings.
     */
    void queryFinishedPrivate( const QgsDatabaseQueryLogEntry &query ) SIP_SKIP;

  signals:

    /**
     * Emitted whenever a database query is started.
     *
     * \note Not available in Python bindings
     */
    void queryStarted( const QgsDatabaseQueryLogEntry &query ) SIP_SKIP;

    /**
     * Emitted whenever a database query has finished executing.
     *
     * \note Not available in Python bindings
     */
    void queryFinished( const QgsDatabaseQueryLogEntry &query ) SIP_SKIP;

  private:

    static bool sEnabled;

};

#ifndef SIP_RUN
///@cond private

/**
 * The QgsDatabaseQueryLogWrapper class is a RIIA wrapper for the query logger.
 */
class QgsDatabaseQueryLogWrapper
{

  public:

    QgsDatabaseQueryLogWrapper( const QString &query, const QString &uri, const QString &provider, const QString &initiatorClass, const QString &origin )
      : mEntry( query )
    {
      mEntry.uri = uri;
      mEntry.origin = origin;
      mEntry.initiatorClass = initiatorClass;
      mEntry.provider = provider;
      QgsDatabaseQueryLog::log( mEntry );
    }

    ~QgsDatabaseQueryLogWrapper( )
    {
      QgsDatabaseQueryLog::finished( mEntry );
    }

    void setFetchedRows( long long fetchedRows )
    {
      mEntry.fetchedRows = fetchedRows;
    }

    void setQuery( const QString &query )
    {
      mEntry.query = query;
    }

    void setError( const QString &error )
    {
      mEntry.error = error;
    }

    void setCanceled( )
    {
      mEntry.canceled = true;
    }

  private:

    QgsDatabaseQueryLogEntry mEntry;

};

///@endcond
#endif

#endif // QGSDBQUERYLOG_H
