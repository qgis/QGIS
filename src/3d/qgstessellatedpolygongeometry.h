/***************************************************************************
  qgstessellatedpolygongeometry.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTESSELLATEDPOLYGONGEOMETRY_H
#define QGSTESSELLATEDPOLYGONGEOMETRY_H

#include "qgsfeatureid.h"
#include "qgspolygon.h"

#include <Qt3DRender/QGeometry>

namespace Qt3DRender
{
  class QBuffer;
}

/**
 * \ingroup 3d
 * Class derived from Qt3DRender::QGeometry that represents polygons tessellated into 3D geometry.
 *
 * Takes a list of polygons as input, internally it does tessellation and writes output to the internal
 * vertex buffer. Optionally it can add "walls" if the extrusion height is non-zero.
 *
 * \since QGIS 3.0
 */
class QgsTessellatedPolygonGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT
  public:
    //! Constructor
    QgsTessellatedPolygonGeometry( QNode *parent = nullptr );

    //! Returns whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    bool invertNormals() const { return mInvertNormals; }
    //! Sets whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    void setInvertNormals( bool invert ) { mInvertNormals = invert; }

    /**
     * Returns whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    bool addBackFaces() const { return mAddBackFaces; }

    /**
     * Sets whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    void setAddBackFaces( bool add ) { mAddBackFaces = add; }

    //! Initializes vertex buffer from given polygons. Takes ownership of passed polygon geometries
    void setPolygons( const QList<QgsPolygon *> &polygons, const QList<QgsFeatureId> &featureIds, const QgsPointXY &origin, float extrusionHeight, const QList<float> &extrusionHeightPerPolygon = QList<float>() );

    //! Returns ID of the feature to which given triangle index belongs (used for picking)
    QgsFeatureId triangleIndexToFeatureId( uint triangleIndex );

  private:

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;

    QVector<QgsFeatureId> mTriangleIndexFids;
    QVector<uint> mTriangleIndexStartingIndices;

    bool mWithNormals = true;
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
};

#endif // QGSTESSELLATEDPOLYGONGEOMETRY_H
