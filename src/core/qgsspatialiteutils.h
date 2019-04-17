/***************************************************************************
                          qgsspatialiteutils.h
                           -------------------
    begin                : Nov, 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALITEUTILS_H
#define QGSSPATIALITEUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgssqliteutils.h"
#include <functional>

/**
 * \ingroup core
 *
 * Closes a spatialite database.
 *
 * \since QGIS 3.0
 */
struct CORE_EXPORT QgsSpatialiteCloser
{

  /**
   * Closes an spatialite \a database.
   */
  void operator()( sqlite3 *database );

  /**
   * Keep track of the spatialite context. Set in open(_v2)
   */
  void *mSpatialiteContext = nullptr;
};

/**
 * \ingroup core
 *
 * Unique pointer for spatialite databases, which automatically closes
 * the database when the pointer goes out of scope or is reset.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT spatialite_database_unique_ptr : public std::unique_ptr< sqlite3, QgsSpatialiteCloser>
{
  public:

    /**
     * Opens the database at the specified file \a path.
     *
     * Returns the sqlite error code, or SQLITE_OK if open was successful.
     */
    int open( const QString &path );

    /**
     * Will close the connection and set the internal pointer to NULLPTR.
     */
    void reset();

    /**
     * It is not allowed to set an arbitrary sqlite3 handle on this object.
     *
     * A dedicated spatialite context (connection) is created when calling open or
     * open_v2. This context needs to be kept together with the handle, hence it is
     * not allowed to reset to a handle with missing context.
     */
    void reset( sqlite3 *handle ) = delete;

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
    sqlite3_statement_unique_ptr prepare( const QString &sql, int &resultCode );
};

#endif // QGSSPATIALITEUTILS_H
