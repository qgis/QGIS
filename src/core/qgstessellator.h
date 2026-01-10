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
class QgsLineString;
class QgsCurve;

#include <QColor>
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
    void setOrigin( const QgsVector3D &origin );

    /**
     * Returns the origin point of the map.
     * \since QGIS 4.0
     */
    QgsVector3D origin() const { return mOrigin; }

    /**
     * Sets scaling and the bounds of the input geometry coordinates.
     * \since QGIS 4.0
     */
    void setBounds( const QgsRectangle &bounds );

    /**
     * Sets whether Z values from the input geometries are ignored (TRUE) or not (FALSE).
     * By default, this is FALSE.
     * \since QGIS 4.0
     */
    void setInputZValueIgnored( bool ignore );

    /**
     * Returns whether Z values from the input geometries are ignored (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    bool isZValueIgnored() const { return mInputZValueIgnored; }

    /**
     * Sets which faces should be generated during extrusion.
     * \since QGIS 4.0
     */
    void setExtrusionFaces( Qgis::ExtrusionFaces faces );

    /**
     * Returns which faces are generated during extrusion.
     * \since QGIS 4.0
     */
    Qgis::ExtrusionFaces extrusionFaces() const { return mExtrusionFaces; }

    /**
     * Sets the rotation of texture UV coordinates (in degrees).
     * \since QGIS 4.0
     */
    void setTextureRotation( float rotation );

    /**
     * Returns the rotation of texture UV coordinates (in degrees).
     * \since QGIS 4.0
     */
    float textureRotation() const { return mTextureRotation; }

    /**
     * Sets whether texture UV coordinates should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setAddTextureUVs( bool addTextureUVs );

    /**
     * Returns whether texture UV coordinates are being added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    bool hasTextureUVs() const { return mAddTextureCoords; }

    /**
     * Sets whether normals should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setAddNormals( bool addNormals );

    /**
     * Returns whether normals are being added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    bool hasNormals() const { return mAddNormals; }

    /**
     * Sets whether back faces should be added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setBackFacesEnabled( bool addBackFaces );

    /**
     * Returns whether back faces are being added to the output data (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    bool hasBackFacesEnabled() const { return mAddBackFaces; }

    /**
     * Sets whether normals should be inverted (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    void setInvertNormals( bool invertNormals );

    /**
     * Returns whether normals are inverted (TRUE) or not (FALSE).
     * \since QGIS 4.0
     */
    bool hasInvertedNormals() const { return mInvertNormals; }

    /**
     * Sets the triangulation algorithm.
     * \since QGIS 4.0
     */
    void setTriangulationAlgorithm( Qgis::TriangulationAlgorithm algorithm );

    /**
     * Returns the algorithm used for triangulation.
     * \since QGIS 4.0
     */
    Qgis::TriangulationAlgorithm triangulationAlgorithm() const { return mTriangulationAlgorithm; }

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
    QVector<float> data() const;

    /**
     * Returns index buffer for the generated points.
     * \since QGIS 4.0
     */
    QVector<uint32_t> indexBuffer() const { return mIndexBuffer; }

    /**
     * Returns vertex buffer for the generated points.
     * \since QGIS 4.0
     */
    QVector<float> vertexBuffer() const;

    //! Returns the number of vertices stored in the output data array
    int dataVerticesCount() const;

    //! Returns size of one vertex entry in bytes
    int stride() const { return mStride; }

    //! Returns size of one index entry in bytes
    int indexStride() const { return sizeof( uint32_t ); }

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

    /**
     * Returns unique vertex count.
     * \since QGIS 4.0
     */
    int uniqueVertexCount() const;

    /**
     * Sets the material colors for data-defined rendering.
     * Colors are not applied to the already added polygons.
     * \since QGIS 4.0
     */
    void setMaterialColors( const QColor &diffuse, const QColor &ambient, const QColor &specular )
    {
      mDiffuseColor = diffuse;
      mAmbientColor = ambient;
      mSpecularColor = specular;
    }

  private:
    struct VertexPoint
    {
      QVector3D position;
      QVector3D normal;
      float u = 0.0f;
      float v = 0.0f;
      QColor diffuseColor;
      QColor ambientColor;
      QColor specularColor;

      inline bool operator==( const VertexPoint &other ) const
      {
        return position == other.position
               && normal == other.normal
               && u == other.u
               && v == other.v
               && diffuseColor == other.diffuseColor
               && ambientColor == other.ambientColor
               && specularColor == other.specularColor;
      }
    };

    friend uint qHash( const VertexPoint &key )
    {
      return qHash( key.position.x() ) ^ qHash( key.position.y() ) ^ qHash( key.position.z() )
             ^ qHash( key.normal.x() ) ^ qHash( key.normal.y() ) ^ qHash( key.normal.z() )
             ^ qHash( key.u ) ^ qHash( key.v ) ^ qHash( key.diffuseColor ) ^ qHash( key.ambientColor ) ^ qHash( key.specularColor );
    }

    QHash<VertexPoint, uint32_t> mVertexBuffer;
    QVector<uint32_t> mIndexBuffer;

    void updateStride();
    void setExtrusionFacesLegacy( int facade );
    void calculateBaseTransform( const QVector3D &pNormal, QMatrix4x4 *base ) const;
    void addTriangleVertices( const std::array<QVector3D, 3> &points, QVector3D pNormal, float extrusionHeight, QMatrix4x4 *transformMatrix, const QgsPoint *originOffset, bool reverse );
    void addVertexPoint( VertexPoint &vertexPoint );
    void makeWalls( const QgsLineString &ring, bool ccw, float extrusionHeight );
    void addExtrusionWallQuad( const QVector3D &pt1, const QVector3D &pt2, float height );
    void ringToEarcutPoints( const QgsLineString *ring, std::vector<std::array<double, 2>> &polyline, QHash<std::array<double, 2>*, float> *zHash );
    std::vector<QVector3D> generateConstrainedDelaunayTriangles( const QgsPolygon *polygonNew );
    std::vector<QVector3D> generateEarcutTriangles( const QgsPolygon *polygonNew );

    QgsVector3D mOrigin = QgsVector3D( 0, 0, 0 );
    bool mAddNormals = false;
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
    bool mAddTextureCoords = false;
    bool mOutputZUp = false;
    QVector<float> mData;
    int mStride = 3 * sizeof( float );
    bool mInputZValueIgnored = false;
    QColor mDiffuseColor = QColor( 0, 0, 0 );
    QColor mAmbientColor = QColor( 0, 0, 0 );
    QColor mSpecularColor = QColor( 0, 0, 0 );
    Qgis::ExtrusionFaces mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof;
    Qgis::TriangulationAlgorithm mTriangulationAlgorithm = Qgis::TriangulationAlgorithm::ConstrainedDelaunay;
    float mTextureRotation = 0.0f;
    float mScale = 1.0f;
    QString mError;

    float mZMin = std::numeric_limits<float>::max();
    float mZMax = -std::numeric_limits<float>::max();
};


#endif // QGSTESSELLATOR_H
