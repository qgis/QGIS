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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QGeometry>
#else
#include <Qt3DCore/QGeometry>
#endif

class Qgs3DSceneExporter;
class QgsPolygon;
class QgsPointXY;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace Qt3DRender
{
  class QBuffer;
}
#else
namespace Qt3DCore
{
  class QBuffer;
}
#endif

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Class derived from Qt3DRender::QGeometry that represents polygons tessellated into 3D geometry.
 *
 * Takes a list of polygons as input, internally it does tessellation and writes output to the internal
 * vertex buffer. Optionally it can add "walls" if the extrusion height is non-zero.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.0
 */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QgsTessellatedPolygonGeometry : public Qt3DRender::QGeometry
#else
class QgsTessellatedPolygonGeometry : public Qt3DCore::QGeometry
#endif
{
    Q_OBJECT
  public:
    //! Constructor
    QgsTessellatedPolygonGeometry( bool _withNormals = true, bool invertNormals = false, bool addBackFaces = false, bool addTextureCoords = false, QNode *parent = nullptr );

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

    /**
     * Sets whether the texture coordinates will be generated
     * \since QGIS 3.16
     */
    void setAddTextureCoords( bool add ) { mAddTextureCoords = add; }

    //! Initializes vertex buffer from given polygons. Takes ownership of passed polygon geometries
    void setPolygons( const QList<QgsPolygon *> &polygons, const QList<QgsFeatureId> &featureIds, const QgsPointXY &origin, float extrusionHeight, const QList<float> &extrusionHeightPerPolygon = QList<float>() );

    /**
     * Initializes vertex buffer (and other members) from data that were already tessellated.
     * This is an alternative to setPolygons() - this method does not do any expensive work in the body.
     * \since QGIS 3.12
     */
    void setData( const QByteArray &vertexBufferData, int vertexCount, const QVector<QgsFeatureId> &triangleIndexFids, const QVector<uint> &triangleIndexStartingIndices );

    /**
     * Returns ID of the feature to which given triangle index belongs (used for picking).
     * In case such triangle index does not match any feature, FID_NULL is returned.
     */
    QgsFeatureId triangleIndexToFeatureId( uint triangleIndex ) const;

    friend class Qgs3DSceneExporter;
  private:

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mTextureCoordsAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
#else
    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QAttribute *mNormalAttribute = nullptr;
    Qt3DCore::QAttribute *mTextureCoordsAttribute = nullptr;
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
#endif

    QVector<QgsFeatureId> mTriangleIndexFids;
    QVector<uint> mTriangleIndexStartingIndices;

    bool mWithNormals = true;
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
    bool mAddTextureCoords = false;
};

#endif // QGSTESSELLATEDPOLYGONGEOMETRY_H
