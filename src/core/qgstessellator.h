/***************************************************************************
  qgstessellator.h
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

#ifndef QGSTESSELLATOR_H
#define QGSTESSELLATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"

class QgsPolygon;
class QgsMultiPolygon;

#include <QVector>
#include <memory>

/**
 * \ingroup core
 * \brief Tessellates polygons into triangles.
 *
 * It is expected that client code will create the tessellator object, then repeatedly call
 * addPolygon() method that will generate triangles, and finally call data() to get final vertex data.
 *
 * Optionally provides extrusion by adding triangles that serve as walls when extrusion height is non-zero.
 *
 */
class CORE_EXPORT QgsTessellator
{
  public:
    QgsTessellator();

    /**
     * \brief Creates tessellator with a specified origin point of the world (in map coordinates)
     * \deprecated QGIS 4.0. Use the default QgsTessellator() constructor and individual setters instead.
     */
    Q_DECL_DEPRECATED QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals = false, bool addBackFaces = false, bool noZ = false,
                                      bool addTextureCoords = false, int facade = 3, float textureRotation = 0.0f ) SIP_DEPRECATED;

    /**
     * Creates tessellator with a specified \a bounds of input geometry coordinates.
     * This constructor allows the tessellator to map input coordinates to a desirable range for numerical
     * stability during calculations.
     *
     * If \a noZ is TRUE, then a 2-dimensional tessellation only will be performed and all z coordinates will be ignored.
     *
     * \since QGIS 3.10
     * \deprecated QGIS 4.0. Use the default QgsTessellator() constructor and individual setters instead.
     */
    Q_DECL_DEPRECATED QgsTessellator( const QgsRectangle &bounds, bool addNormals, bool invertNormals = false, bool addBackFaces = false, bool noZ = false,
                                      bool addTextureCoords = false, int facade = 3, float textureRotation = 0.0f ) SIP_DEPRECATED;

    /**
     * Sets the origin point of the map.
     * \since QGIS 4.0
     */
    void setOrigin( QgsVector3D origin );

    /**
     * Sets scaling and the bounds of the input geometry coordinates.
     * \since QGIS 4.0
     */
    void setBounds( const QgsRectangle &bounds );

    /**
     * Sets whether extrusion is enabled (TRUE) or disabled (FALSE).
     * By default, extrusion is disabled.
     * \since QGIS 4.0
     */
    void setExtrusionEnabled( bool enabled );

    /**
     * Sets which faces should be generated during extrusion.
     * \since QGIS 4.0
     */
    void setExtrusionFaces( Qgis::ExtrusionFaces faces );

    /**
     * Sets the rotation of generated faces.
     * \since QGIS 4.0
     */
    void setTextureRotation( float rotation );

    /**
     * Sets whether texture UV coordinates should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setAddTextureUVs( bool addTextureUVs );

    /**
     * Sets whether normals should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setAddNormals( bool addNormals );

    /**
     * Sets whether back faces should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setAddBackFaces( bool addBackFaces );

    /**
     * Sets whether normals should be inverted (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setInvertNormals( bool invertNormals );

    /**
     * Sets whether the "up" direction should be the Z axis on output (TRUE),
     * otherwise the "up" direction will be the Y axis (FALSE). The default
     * value is FALSE (to keep compatibility for existing tessellator use cases).
     * \since QGIS 3.42
     */
    void setOutputZUp( bool zUp ) { mOutputZUp = zUp; }

    /**
     * Returns whether the "up" direction should be the Z axis on output (TRUE),
     * otherwise the "up" direction will be the Y axis (FALSE). The default
     * value is FALSE (to keep compatibility for existing tessellator use cases).
     * \since QGIS 3.42
     */
    bool isOutputZUp() const { return mOutputZUp; }

    //! Tessellates a triangle and adds its vertex entries to the output data array
    void addPolygon( const QgsPolygon &polygon, float extrusionHeight );

    /**
     * Returns array of triangle vertex data
     *
     * Vertice coordinates are stored as (x, z, -y)
     */
    QVector<float> data() const { return mData; }

    //! Returns the number of vertices stored in the output data array
    int dataVerticesCount() const;

    //! Returns size of one vertex entry in bytes
    int stride() const { return mStride; }

    /**
     * Returns the triangulation as a multipolygon geometry.
     */
    std::unique_ptr< QgsMultiPolygon > asMultiPolygon() const SIP_SKIP;

    /**
     * Returns minimal Z value of the data (in world coordinates)
     * \since QGIS 3.12
     */
    float zMinimum() const { return mZMin; }

    /**
     * Returns maximal Z value of the data (in world coordinates)
     * \since QGIS 3.12
     */
    float zMaximum() const { return mZMax; }

    /**
     * Returns a descriptive error string if the tessellation failed.
     *
     * \since QGIS 3.34
     */
    QString error() const { return mError; }

  private:
    void updateStride();

    void setExtrusionFacesLegacy( int facade );
    void calculateBaseTransform( const QVector3D &pNormal, QMatrix4x4 *base ) const;
    void addTriangleVertices( const std::array<QVector3D, 3> &points, QVector3D pNormal, float extrusionHeight, QMatrix4x4 *transformMatrix, const QgsPoint *originOffset, bool reverse );
    std::vector<QVector3D> generateConstrainedDelaunayTriangles( const QgsPolygon *polygonNew );

    QgsVector3D mOrigin = QgsVector3D( 0, 0, 0 );
    bool mAddNormals = false;
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
    bool mAddTextureCoords = false;
    bool mOutputZUp = false;
    QVector<float> mData;
    int mStride = 3 * sizeof( float );
    bool mNoZ = false;
    Qgis::ExtrusionFaces mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof;
    float mTextureRotation = 0.0f;
    float mScale = 1.0;
    QString mError;

    float mZMin = std::numeric_limits<float>::max();
    float mZMax = -std::numeric_limits<float>::max();
};


#endif // QGSTESSELLATOR_H
