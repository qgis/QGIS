/***************************************************************************
                          qgssqliteutils.h
                           -------------------
    begin                : Nov, 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLITEUTILS_H
#define QGSSQLITEUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <memory>
#include <QString>

struct sqlite3;
struct sqlite3_stmt;
class QVariant;

#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * Closes a sqlite3 database.
 *
 * \since QGIS 3.0
 */
struct CORE_EXPORT QgsSqlite3Closer
{

  /**
   * Closes an sqlite \a database.
   */
  void operator()( sqlite3 *database );
};

/**
 * Finalizes an sqlite3 statement.
 */
struct CORE_EXPORT  QgsSqlite3StatementFinalizer
{

  /**
   * Finalizes an sqlite3 \a statement.
   */
  void operator()( sqlite3_stmt *statement );
};

/**
 * \ingroup core
 *
 * Unique pointer for sqlite3 prepared statements, which automatically finalizes
 * the statement when the pointer goes out of scope or is reset.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT sqlite3_statement_unique_ptr : public std::unique_ptr< sqlite3_stmt, QgsSqlite3StatementFinalizer>
{
  public:

    /**
     * Steps to the next record in the statement, returning the sqlite3 result code.
     */
    int step();

    /**
     * Returns the name of \a column.
     */
    QString columnName( int column ) const;

    /**
     * Returns the column value from the current statement row as a string.
     */
    QString columnAsText( int column ) const;

    /**
     * Gets column value from the current statement row as a long long integer (64 bits).
     */
    qlonglong columnAsInt64( int column ) const;

    /**
     * Gets column value from the current statement row as a double.
     */
    double columnAsDouble( int column ) const;

    /**
     * Gets the number of columns that this statement returns.
     */
    int columnCount() const;
};


/**
 * \ingroup core
 *
 * Unique pointer for sqlite3 databases, which automatically closes
 * the database when the pointer goes out of scope or is reset.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT sqlite3_database_unique_ptr : public std::unique_ptr< sqlite3, QgsSqlite3Closer>
{
  public:

    /**
     * Opens the database at the specified file \a path.
     *
     * Returns the sqlite error code, or SQLITE_OK if open was successful.
     */
    int open( const QString &path );

    /**
     * Opens the database at the specified file \a path.
     *
     * Returns the sqlite error code, or SQLITE_OK if open was successful.
     */
    int open_v2( const QString &path, int flags, const char *zVfs );

    /**
     * Returns the most recent error message encountered by the database.
     */
    QString errorMessage() const;

    /**
     * Prepares a \a sql statement, returning the result. The \a resultCode
     * argument will be filled with the sqlite3 result code.
     */
    sqlite3_statement_unique_ptr prepare( const QString &sql, int &resultCode SIP_OUT ) const;

    /**
     * Executes the \a sql command in the database. Multiple sql queries can be run within
     * one single command.
     * Errors are reported to \a errorMessage.
     * Returns SQLITE_OK in case of success or an sqlite error code.
     *
     * \since QGIS 3.6
     */
    int exec( const QString &sql, QString &errorMessage SIP_OUT ) const;
};

/**
 * Wraps sqlite3_mprintf() by automatically freeing the memory.
 * \note not available in Python bindings.
 * \since QGIS 3.2
 */
QString CORE_EXPORT QgsSqlite3Mprintf( const char *format, ... );

#endif

/**
 * Contains utilities for working with Sqlite data sources.
 * \ingroup core
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsSqliteUtils
{
  public:

    /**
     * Returns a quoted string \a value, surround by ' characters and with special
     * characters correctly escaped.
     */
    static QString quotedString( const QString &value );

    /**
     * Returns a properly quoted version of \a identifier.
     *
     * \since QGIS 3.6
     */
    static QString quotedIdentifier( const QString &identifier );

    /**
     * Returns a properly quoted and escaped version of \a value
     * for use in SQL strings.
     *
     * \since QGIS 3.6
     */
    static QString quotedValue( const QVariant &value );

    /**
     * Returns a string list of SQLite (and spatialite) system tables
     *
     * \since QGIS 3.8
     */
    static QStringList systemTables();
};

#endif // QGSSQLITEUTILS_H
