/***************************************************************************
  qgstilecache.h
  --------------------------------------
  Date                 : September 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILECACHE_H
#define QGSTILECACHE_H

#include "qgis_core.h"
#include <QCache>
#include <QMutex>

class QImage;
class QUrl;

#define SIP_NO_FILE

/**
 * A simple tile cache implementation. Tiles are cached according to their URL.
 * There is a small in-memory cache and a secondary caching in the local disk.
 * The in-memory cache is there to save CPU time otherwise wasted to read and
 * uncompress data saved on the disk.
 *
 * The class is thread safe (its methods can be called from any thread).
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.8.0
 */
class CORE_EXPORT QgsTileCache
{
  public:

    //! Add a tile image with given URL to the cache
    static void insertTile( const QUrl &url, const QImage &image );

    /**
     * Try to access a tile and load it into "image" argument
     * \returns true if the tile exists in the cache
     */
    static bool tile( const QUrl &url, QImage &image );

    //! how many tiles are stored in the in-memory cache
    static int totalCost() { QMutexLocker locker( &sTileCacheMutex ); return sTileCache.totalCost(); }
    //! how many tiles can be stored in the in-memory cache
    static int maxCost() { QMutexLocker locker( &sTileCacheMutex ); return sTileCache.maxCost(); }

  private:
    //! in-memory cache
    static QCache<QUrl, QImage> sTileCache;
    //! mutex to protect the in-memory cache
    static QMutex sTileCacheMutex;
};

#endif // QGSTILECACHE_H
