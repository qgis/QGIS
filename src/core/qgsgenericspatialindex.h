/***************************************************************************
  qgsgenericspatialindex.h
  ------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGENERICSPATIALINDEX_H
#define QGSGENERICSPATIALINDEX_H

#include "qgis_core.h"
#include "qgsspatialindexutils.h"

#include <memory>
#include <QMutex>

#define SIP_NO_FILE

#include <functional>

class QgsRectangle;

/**
 * \ingroup core
 * \class QgsGenericSpatialIndex
 *
 * A generic rtree spatial index based on a libspatialindex backend.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.12
 */
template <typename T>
class CORE_EXPORT QgsGenericSpatialIndex
{
  public:

    QgsGenericSpatialIndex();
    ~QgsGenericSpatialIndex();

    /**
     * Inserts new \a data into the spatial index, with the specified \a bounds.
     *
     * Ownership of \a data is not transferred, and it is the caller's responsibility to ensure that
     * it exists for the lifetime of the spatial index.
     */
    bool insert( const T *data, const QgsRectangle &bounds );

    /**
     * Removes existing \a data from the spatial index, with the specified \a bounds.
     *
     * \a data is not deleted, and it is the caller's responsibility to ensure that
     * it is appropriately cleaned up.
     */
    bool remove( const T *data, const QgsRectangle &bounds );

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( const T *data )> &callback ) const;

  private:

    std::unique_ptr< SpatialIndex::IStorageManager > mStorageManager;
    std::unique_ptr< SpatialIndex::ISpatialIndex > mRTree;

    mutable QMutex mMutex;

    qint64 mNextId = 1;
    QHash< qint64, const T * > mIdToData;
    QHash< const T *, qint64 > mDataToId;

};

#endif // QGSGENERICSPATIALINDEX_H
