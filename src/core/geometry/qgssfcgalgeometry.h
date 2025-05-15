/***************************************************************************
                         qgssfcgalGeometry.h
                         -------------------
    begin                : May 2025
    copyright            : (C) 2025 by Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
    email                : jean dot felder at oslandia dot com
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSGCGAL_GEOMETRY_H
#define QGSSGCGAL_GEOMETRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractgeometry.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgspolyhedralsurface.h"
#include "qgstriangulatedsurface.h"
#include "qgstriangle.h"
#include "qgssfcgalengine.h"
#include "qgslogger.h"

/**
 * Wraps SFCGAL geometry object and its QgsAbstractGeometry source
 * \ingroup core
 * \class QgsSfcgalGeometry
 * \brief SfcgalGeometry geometry type.
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsSfcgalGeometry : public QgsAbstractGeometry
{
  public:
    //! Constructor for an empty sfcgalGeometry geometry.
    QgsSfcgalGeometry() SIP_HOLDGIL;

    //! Constructor with QgsAbstractGeometry and optional matching SFCGAL geometry
    QgsSfcgalGeometry( std::unique_ptr<QgsAbstractGeometry> &qgsGeom, sfcgal::shared_geom sfcgalGeom = nullptr );

    //! Constructor with QgsGeometry and optional matching SFCGAL geometry
    QgsSfcgalGeometry( const QgsGeometry &qgsGeom, sfcgal::shared_geom sfcgalGeom = nullptr );

    //! Copy constructor
    QgsSfcgalGeometry( const QgsSfcgalGeometry &geom );

    //! Returns the underlying SFCGAL geometry
    sfcgal::shared_geom sfcgalGeometry() const { return mSfcgalGeom; }

    //! Returns the underlying QGIS geometry
    const QgsAbstractGeometry *delegatedGeometry() const { return mQgsGeom.get(); }

    // QgsAbstractGeometry overrides: delegates to underlying QGIS geometry
    QString geometryType() const override SIP_HOLDGIL;
    QgsAbstractGeometry *clone() const override;
    void clear() override;
    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = -1 ) const override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    QgsAbstractGeometry *createEmptyWithSameType() const override;
    bool operator==( const QgsAbstractGeometry &other ) const override;
    bool operator!=( const QgsAbstractGeometry &other ) const override;
    bool fuzzyEqual( const QgsAbstractGeometry &other, double epsilon ) const override;
    bool fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon ) const override;
    QgsBox3D boundingBox3D() const override;
    int dimension() const override;
    void normalize() override;
    bool fromWkt( const QString &wkt ) override;
    QDomElement asGml2( QDomDocument &doc, int precision, const QString &ns, AxisOrder axisOrder ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision, const QString &ns, AxisOrder axisOrder ) const override;
    QString asKml( int precision ) const override;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ ) override;
    void transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale ) override;
    void draw( QPainter &p ) const override;
    QPainterPath asQPainterPath() const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool nextVertex( QgsVertexId &id, QgsPoint &vertex ) const override;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const override;
    QgsCoordinateSequence coordinateSequence() const override;
    QgsPoint vertexAt( QgsVertexId id ) const override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const override;
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;
    double segmentLength( QgsVertexId startVertex ) const override;
    QgsAbstractGeometry *toCurveType() const override;
    QgsAbstractGeometry *snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing, bool removeRedundantPoints = false ) const override;
    bool removeDuplicateNodes( double epsilon, bool useZValues ) override;
    double vertexAngle( QgsVertexId vertex ) const override;
    int vertexCount( int part, int ring ) const override;
    int ringCount( int part ) const override;
    int partCount() const override;
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;
    bool isValid( QString &error, Qgis::GeometryValidityFlags flags ) const override;
    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback ) override;
    QgsAbstractGeometry *simplifyByDistance( double tolerance ) const override;
    QgsPoint centroid() const override;
    bool isEmpty() const override;
    double area() const override;
    double length() const override;
    // End of QgsAbstractGeometry overrides

    /**
     * Checks this geometry is simple.
     *
     * \see QgsSfcgalEngine::isSimple
     *
     * \param errorMsg Error message returned by SFGCAL
     */
    bool isSimple( QString *errorMsg = nullptr ) const;

    /**
     * Calculates the centroid of this geometry.

     * \param errorMsg Error message returned by SFGCAL
     */
    QgsPoint centroid( QString *errorMsg ) const;

    /**
     * Translate this geometry by vector \a translation.
     *
     * \param translation translation vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *translate( const QgsPoint &translation, QString *errorMsg = nullptr ) const;

    /**
     * Scale this geometry by vector \a scaleFactor.
     *
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *scale( const QgsPoint &scaleFactor, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr ) const;

    /**
     * 2D Rotate this geometry around point \center by angle \angle
     *
     * \param angle rotation angle in radians
     * \param center rotation center
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *rotate2D( double angle, const QgsPoint &center, QString *errorMsg = nullptr ) const;

    /**
     * 3D Rotate this geometry around axis \a axisVector by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr ) const;

    /**
     * Checks if \a geom intersects this.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    bool intersects( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr ) const;

    /**
     * Calculate the intersection of this and \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the intersection results
     */
    QgsSfcgalGeometry *intersection( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the combination of this and \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *combine( const QVector<const QgsAbstractGeometry *> &geomList, QString *errorMsg ) const;

    /**
     * Calculate the difference of this and \a geom.
     *
     * \param geom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the difference results
     */
    QgsSfcgalGeometry *difference( const QgsAbstractGeometry *geom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Triangulates this geometry using constraint 2D Delaunay Triangulation (keep Z if defined)
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *triangulate( QString *errorMsg = nullptr ) const;

    /**
     * Calculate the convex hull (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *convexhull( QString *errorMsg = nullptr ) const;

    /**
     * Calculate the envelope (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *envelope( QString *errorMsg = nullptr ) const;

    /**
     * Calculate a 3D buffer where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * It is limited to Point and LineString.
     * If the operation fails, a null pointer is returned.
     *
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle3D the type of buffer to compute
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg = nullptr ) const;

    /**
     * Calculate a 2D buffer where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * If the operation fails, a null pointer is returned.
     *
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle the type of buffer to compute. Only round is supported.
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg ) const;

    /**
     * Simplifies a geometry using the CGAL algorithm
     *
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *simplify( double tolerance, bool preserveTopology, QString *errorMsg = nullptr ) const;

    /**
     * Calculate an extrusion of the original geometry.
     * If the operation fails, a null pointer is returned.
     *
     * \param extrusion extrusion vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry *extrude( const QgsPoint &extrusion, QString *errorMsg = nullptr ) const;

#ifndef SIP_RUN
    /**
     * Cast the \a geom to the exact underlying QGIS geometry.
     * Should be used, for example, by qgsgeometry_cast<QgsPoint *>( geometry ) or by qgsgeometry_cast<QgsPolygon *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     */
    inline static const QgsAbstractGeometry *cast( const QgsAbstractGeometry *geom )
    {
      if ( geom && dynamic_cast<const QgsSfcgalGeometry *>( geom ) )
      {
        const QgsSfcgalGeometry *sfcgalGeom = dynamic_cast<const QgsSfcgalGeometry *>( geom );
        const Qgis::WkbType type = QgsWkbTypes::flatType( sfcgalGeom->wkbType() );
        switch ( type )
        {
          case Qgis::WkbType::Point:
            return QgsPoint::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::LineString:
            return QgsLineString::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::CircularString:
            return QgsCircularString::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::CompoundCurve:
            return QgsCompoundCurve::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::Polygon:
            return QgsPolygon::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::CurvePolygon:
            return QgsCurvePolygon::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::MultiLineString:
            return QgsMultiLineString::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::MultiPolygon:
            return QgsMultiPolygon::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::MultiPoint:
            return QgsMultiPoint::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::MultiCurve:
            return QgsMultiCurve::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::MultiSurface:
            return QgsMultiSurface::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::GeometryCollection:
            return QgsGeometryCollection::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::Triangle:
            return QgsTriangle::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::PolyhedralSurface:
            return QgsPolyhedralSurface::cast( sfcgalGeom->delegatedGeometry() );
          case Qgis::WkbType::TIN:
            return QgsTriangulatedSurface::cast( sfcgalGeom->delegatedGeometry() );
          default:
            return nullptr;
        }
      }
      return nullptr;
    }
#endif

  protected:
    sfcgal::shared_geom mSfcgalGeom;
    std::unique_ptr<QgsAbstractGeometry> mQgsGeom;

    mutable bool mHasCachedValidity = false;
    mutable QString mValidityFailureReason;

  protected:
    int compareToSameClass( const QgsAbstractGeometry *other ) const override;
    void clearCache() const override;

  private:
    void resetQgsGeometry();
};


#endif // QGSSGCGAL_GEOMETRY_H
