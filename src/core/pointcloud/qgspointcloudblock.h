/***************************************************************************
                         qgspointcloudblock.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDBLOCK_H
#define QGSPOINTCLOUDBLOCK_H

#include "qgis.h"
#include "qgis_core.h"
#include <QPair>
#include <QString>
#include <QVector>
#include <QByteArray>

#include "qgspointcloudattribute.h"

/**
 * \ingroup core
 * \brief Base class for storing raw data from point cloud nodes
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudBlock
{
  public:
    //! Ctor
    QgsPointCloudBlock( int count,
                        const QgsPointCloudAttributeCollection &attributes,
                        const QByteArray &data, const QgsVector3D &scale, const QgsVector3D &offset );

    virtual ~QgsPointCloudBlock() = default;

    /**
     * Clones the QgsPointCloudBlock returning a new copy.
     * Caller takes ownership of the returned object.
     * \since QGIS 3.36
     */
    QgsPointCloudBlock *clone() const SIP_FACTORY;

    //! Returns raw pointer to data
    const char *data() const;

    //! Returns number of points that are stored in the block
    int pointCount() const;

    /**
     * Returns the total size of each individual point record.
     *
     * \since QGIS 3.26
     */
    int pointRecordSize() const { return mRecordSize; }

    //! Returns the attributes that are stored in the data block, along with their size
    QgsPointCloudAttributeCollection attributes() const;

    //! Returns the custom scale of the block.
    QgsVector3D scale() const;

    //! Returns the custom offset of the block.
    QgsVector3D offset() const;

    /**
     * Changes the number of points in the block.
     * This is used in order to remove all points after point \a size.
     * If a \a size larger than \a pointCount() is used, data for the new points will be uninitialized
     *
     * \since QGIS 3.26
     */
    void setPointCount( int size );

  private:

    int mPointCount = 0;
    QgsPointCloudAttributeCollection mAttributes;
    int mRecordSize = 0;
    QByteArray mStorage;
    QgsVector3D mScale, mOffset;
};


#endif // QGSPOINTCLOUDBLOCK_H
