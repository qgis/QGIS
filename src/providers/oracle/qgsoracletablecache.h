/***************************************************************************
                             qgsoraclestablecache.h
                              -------------------
begin                : April 2014
copyright            : (C) 2014 by Martin Dobias
email                : wonder.sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLETABLECACHE_H
#define QGSORACLETABLECACHE_H

#include <sqlite3.h>

#include <QFlags>

#include "qgsoracleconn.h"

/**
 * This class contains routines for local caching of listing of layers, so the add Oracle
 * layer dialog does not need to discover the tables every time the user wants to add a layer.
 *
 * The cached entries are stored in SQLite database in QGIS user directory (usually ~/.qgis2).
 * The database can be used for other data sources, too. Each saved connection's list is stored
 * in one table "oracle_xyz" (where xyz is the name of the connection). There is one meta table
 * "meta_oracle" which has a list of cached connections and the combination of flags used for
 * the list. The cached entries are used only in case the flags are exactly the same.
 *
 */
class QgsOracleTableCache
{
  public:

    enum CacheFlag
    {
      OnlyLookIntoMetadataTable = 0x01,
      OnlyLookForUserTables     = 0x02,
      UseEstimatedTableMetadata = 0x04,
      OnlyExistingGeometryTypes = 0x08,
      AllowGeometrylessTables   = 0x10
    };
    Q_DECLARE_FLAGS( CacheFlags, CacheFlag )

    //! Return name of the file used for the cached entries
    static QString cacheDatabaseFilename();

    //! check whether the given connection is cached (with equal flags)
    static bool hasCache( const QString& connName, CacheFlags flags );

    //! Store the given list of entries (layers) into the cache. Returns true on success.
    static bool saveToCache( const QString& connName, CacheFlags flags, const QVector<QgsOracleLayerProperty>& layers );

    //! Try to load cached entries for the given connection and its flags. Returns true on success.
    static bool loadFromCache( const QString& connName, CacheFlags flags, QVector<QgsOracleLayerProperty>& layers );

    //! Remove cached entries for given connection. Returns true on success.
    static bool removeFromCache( const QString& connName );

    //! Rename cached connection (useful when an existing connection gets renamed). Returns true on success.
    static bool renameConnectionInCache( const QString& oldName, const QString& newName );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsOracleTableCache::CacheFlags )

#endif // QGSORACLETABLECACHE_H
