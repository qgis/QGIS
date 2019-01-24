/***************************************************************************
    qgsmeshspatialindex.h
    ---------------------
    begin                : January 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHSPATIALINDEX_H
#define QGSMESHSPATIALINDEX_H

#include "qgis_sip.h"

class QgsRectangle;
class QgsPointXY;
class QgsFeedback;
struct QgsMesh;

#include "qgis_core.h"
#include <QList>
#include <QSharedDataPointer>

class QgsMeshSpatialIndexData;

/**
 * \ingroup core
 * \class QgsMeshSpatialIndex
 *
 * A spatial index for QgsMeshFace objects.
 *
 * QgsMeshSpatialIndex objects are implicitly shared and can be inexpensively copied.
 *
 * \note While the underlying libspatialindex is not thread safe on some platforms, the QgsMeshSpatialIndex
 * class implements its own locks and accordingly, a single QgsMeshSpatialIndex object can safely
 * be used across multiple threads
 *
 * \see QgsSpatialIndex, which is for vector features
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMeshSpatialIndex
{
  public:

    /**
     * Constructor for QgsSpatialIndex. Creates an empty R-tree index.
     */
    QgsMeshSpatialIndex();

    /**
     * Constructor - creates R-tree and bulk loads faces from the specified mesh
     *
     * The optional \a feedback object can be used to allow cancelation of bulk face loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     */
    explicit QgsMeshSpatialIndex( const QgsMesh &triangularMesh, QgsFeedback *feedback = nullptr );

    //! Copy constructor
    QgsMeshSpatialIndex( const QgsMeshSpatialIndex &other );

    //! Destructor finalizes work with spatial index
    ~QgsMeshSpatialIndex();

    //! Implement assignment operator
    QgsMeshSpatialIndex &operator=( const QgsMeshSpatialIndex &other );

    /**
     * Returns a list of face ids with a bounding box which intersects the specified \a rectangle.
     *
     * \note The intersection test is performed based on the face bounding boxes only, so it is necessary
     * to manually test the returned faces for exact geometry intersection when required.
     */
    QList<int> intersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns nearest neighbors to a \a point. The number of neighbours returned is specified
     * by the \a neighbours argument.
     *
     * \note The nearest neighbour test is performed based on the face bounding boxes only,
     * so this method is not guaranteed to return the actual closest neighbours.
     */
    QList<int> nearestNeighbor( const QgsPointXY &point, int neighbors ) const;

  private:
    QSharedDataPointer<QgsMeshSpatialIndexData> d;
};

#endif //QGSMESHSPATIALINDEX_H
