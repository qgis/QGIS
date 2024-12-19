/***************************************************************************
                         qgspointcloudsubindex.h
                         -----------------------
    begin                : March 2023
    copyright            : (C) 2023 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDSUBINDEX_H
#define QGSPOINTCLOUDSUBINDEX_H

#include <memory>
#include <QString>

#include "qgspointcloudindex.h"
#include "qgsgeometry.h"
#include "qgsrange.h"

///@cond PRIVATE
#define SIP_NO_FILE


/**
 * \brief Represents an individual index and metadata for the virtual point cloud data provider.
 *
 * The index is initially NULLPTR until the virtual point cloud data provider explicitly loads the uri.
 *
 * \since QGIS 3.32
 */
class QgsPointCloudSubIndex
{
  public:
    //! Constructor
    QgsPointCloudSubIndex( const QString &uri, const QgsGeometry &geometry, const QgsRectangle &extent, const QgsDoubleRange &zRange, qint64 count )
      : mUri( uri )
      , mExtent( extent )
      , mGeometry( geometry )
      , mPointCount( count )
      , mZRange( zRange )
    {
    }

    //! Returns the point cloud index. May be NULLPTR if not loaded.
    QgsPointCloudIndex index() const { return mIndex; }

    //! Sets the point cloud index to \a index.
    void setIndex( QgsPointCloudIndex index ) { mIndex = index; }

    //! Returns the uri for this sub index
    QString uri() const { return mUri; }

    //! Returns the extent for this sub index in the index's crs coordinates.
    QgsRectangle extent() const { return mExtent; }

    //! Returns the elevation range for this sub index hoping it's in meters.
    QgsDoubleRange zRange() const { return mZRange; }

    /**
     * Returns the bounds of the sub index in the index's crs coordinates as a multi polygon geometry.
     * This can be the same as the extent or a more detailed geometry like a convex hull if available.
     */
    QgsGeometry polygonBounds() const { return mGeometry; }

    //! The number of points contained in the index.
    qint64 pointCount() const { return mPointCount; }

  private:
    QgsPointCloudIndex mIndex = QgsPointCloudIndex( nullptr );
    QString mUri;
    QgsRectangle mExtent;
    QgsGeometry mGeometry;
    qint64 mPointCount = 0;
    QgsDoubleRange mZRange;
};

///@endcond
#endif // QGSPOINTCLOUDSUBINDEX_H
