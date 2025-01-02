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
#include "qgsmeshdataprovider.h"

class QgsMeshSpatialIndexData;

/**
 * \ingroup core
 * \class QgsMeshSpatialIndex
 *
 * \brief A spatial index for QgsMeshFace or QgsMeshEdge objects.
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
     * Constructor - creates R-tree and bulk loads faces or edges from the specified mesh
     *
     * Not implemented to construct R-tree for vertices
     * Since QGIS 3.14 possibility to create R-tree for edges
     *
     * The optional \a feedback object can be used to allow cancellation of bulk face loading. Ownership
     * of \a feedback is not transferred, and callers must take care that the lifetime of feedback exceeds
     * that of the spatial index construction.
     */
    explicit QgsMeshSpatialIndex( const QgsMesh &mesh, QgsFeedback *feedback = nullptr, QgsMesh::ElementType elementType = QgsMesh::ElementType::Face );

    QgsMeshSpatialIndex( const QgsMeshSpatialIndex &other );

    //! Destructor finalizes work with spatial index
    ~QgsMeshSpatialIndex();

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

    /**
     * Returns the type of mesh elements that are indexed
     *
     * \since QGIS 3.14
     */
    QgsMesh::ElementType elementType() const;

    /**
     * Adds a face with \a faceIndex from the \a mesh in the spatial index
     */
    void addFace( int faceIndex, const QgsMesh &mesh );

    /**
     * Removes a face with \a faceIndex from the \a mesh in the spatial index
     */
    void removeFace( int faceIndex, const QgsMesh &mesh );


  private:
    QgsMesh::ElementType mElementType = QgsMesh::ElementType::Face;
    QSharedDataPointer<QgsMeshSpatialIndexData> d;
};

#endif //QGSMESHSPATIALINDEX_H
