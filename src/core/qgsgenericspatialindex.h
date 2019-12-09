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

template <typename T>
class CORE_EXPORT QgsGenericSpatialIndex
{
  public:

    QgsGenericSpatialIndex();

    ~QgsGenericSpatialIndex();

    bool insertData( const T *data, const QgsRectangle &bounds );
    bool deleteData( const T *data, const QgsRectangle &bounds );

    bool intersects( const QgsRectangle &rectangle, const std::function< bool( const T *data )> &callback ) const;

  private:

    std::unique_ptr< SpatialIndex::IStorageManager > mStorageManager;
    std::unique_ptr< SpatialIndex::ISpatialIndex > mRTree;

    mutable QMutex mMutex;

    qint64 mNextId = 1;
    QHash< qint64, const T * > mIdToData;
    QHash< const T *, qint64 > mDataToId;

};

#endif // QGSGENERICSPATIALINDEX_H
