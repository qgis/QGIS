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

#ifdef WITH_SFCGAL
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
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsSfcgalGeometry
{
  public:
    //! Constructor for an empty sfcgalGeometry geometry.
    QgsSfcgalGeometry();

    /**
     * Constructor with SFCGAL shared ptr
     * Will copy the shared ptr.
     */
    SIP_SKIP QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom );

    /**
     * Constructor with QgsAbstractGeometry unique_ptr.
     * Will not take ownership.
     */
    QgsSfcgalGeometry( std::unique_ptr<QgsAbstractGeometry> &qgsGeom );

    //! Constructor with QgsAbstractGeometry reference.
    QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom );

    //! Constructor with QgsGeometry reference.
    QgsSfcgalGeometry( const QgsGeometry &qgsGeom );

    //! Constructor from WKT
    QgsSfcgalGeometry( const QString &wkt, QString *errorMsg = nullptr );

    //! Copy constructor
    QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom );

    /**
     * Returns the underlying SFCGAL geometry
     * This operation is always fast, as the SFCGAL geometry representation is maintained for the lifetime of the QgsSfcgalGeometry object.
     */
    SIP_SKIP sfcgal::shared_geom sfcgalGeometry() const { return mSfcgalGeom; }

    /**
     * Returns the geometry converted to a QGIS geometry object.
     * This method is slow to call, as it always involves re-conversion of the underlying SFCGAL geometry object.
     */
    std::unique_ptr<QgsAbstractGeometry> asQgisGeometry( QString *errorMsg = nullptr ) const;

    /**
     * Returns type of the geometry as a WKB type (point / linestring / polygon etc.)
     * \param errorMsg Error message returned by SFGCAL
     */
    Qgis::WkbType wkbType( QString *errorMsg = nullptr ) const;

    /**
     * Returns type of the geometry as a OGC string in CamelCase
     * \param errorMsg Error message returned by SFGCAL
     */
    QString geometryType( QString *errorMsg = nullptr ) const SIP_HOLDGIL;

    /**
     * Clones the geometry by performing a deep copy
     */
    std::unique_ptr<QgsSfcgalGeometry> clone() const;

    /**
     * Creates a new geometry from a WKB byte pointer
     * \param wkbPtr WKB byte pointer
     * \param errorMsg Error message returned by SFGCAL
     */
    bool fromWkb( const QgsConstWkbPtr &wkbPtr, QString *errorMsg = nullptr );

    /**
     * Export the geometry as WKB
     *
     * \param flags argument specifies flags controlling WKB export behavior (since QGIS 3.14).
     * \param errorMsg Error message returned by SFGCAL
     */
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags(), QString *errorMsg = nullptr ) const;

    /**
     * Creates a new geometry from a WKT string.
     * \param wkt WTK string
     * \param errorMsg Error message returned by SFGCAL
     */
    bool fromWkt( const QString &wkt, QString *errorMsg = nullptr );

    /**
     * Export the geometry as WKT
     *
     * \param precision Floating point precision for WKT coordinates. Setting to -1 yields rational number WKT (not decimal) f.e. "Point(1/3, 1/6, 1/4)".
     * \param errorMsg Error message returned by SFGCAL
     */
    QString asWkt( int precision = -1, QString *errorMsg = nullptr ) const;

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \param errorMsg Error message returned by SFGCAL
     * \returns boundary for geometry. May be NULLPTR for some geometry types.
     */
    std::unique_ptr<QgsSfcgalGeometry> boundary( QString *errorMsg = nullptr ) const SIP_FACTORY;

    /**
     * Returns true if this == other geometry
     * \param other geometry to perform the operation
     */
    bool operator==( const QgsSfcgalGeometry &other ) const;

    /**
     * Returns true if this != other geometry
     * \param other geometry to perform the operation
     */
    bool operator!=( const QgsSfcgalGeometry &other ) const;

    /**
     * Returns true if this == other geometry modulo \a epsilon distance
     * \param other geometry to perform the operation
     * \param epsilon tolerance
     * \param errorMsg Error message returned by SFGCAL
     */
    bool fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon, QString *errorMsg = nullptr ) const;

    /**
     * Returns the 3D bounding box for the geometry.
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsBox3D boundingBox3D( QString *errorMsg = nullptr ) const;

    /**
     * Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     * \param errorMsg Error message returned by SFGCAL
     */
    int dimension( QString *errorMsg = nullptr ) const;

    /**
     * Returns the \a geom part count.
     *
     * - POINT, TRIANGLE, LINESTRING: vertex number
     * - POLYGON, SOLID, POLYHEDRALSURFACE, TRIANGULATEDSURFACE: ring or patch or shell number
     * - MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, MULTISOLID, GEOMETRYCOLLECTION: number of geom in collection
     *
     * \param errorMsg pointer to QString to receive the error message if any
     */
    int partCount( QString *errorMsg = nullptr ) const;

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value (existing Z values remains unchanged).
     * \return true if success
     * \param zValue z value to use
     * \param errorMsg pointer to QString to receive the error message if any
     */
    bool addZValue( double zValue = 0, QString *errorMsg = nullptr );

    /**
     * Adds a m-dimension to the geometry, initialized to a preset value (existing M values remains unchanged).
     * \return true if success
     * \param mValue m value to use
     * \param errorMsg pointer to QString to receive the error message if any
     */
    bool addMValue( double mValue = 0, QString *errorMsg = nullptr );

    /**
     * Drops the z coordinate of the geometry
     * \return true if success
     * \param errorMsg pointer to QString to receive the error message if any
     */
    bool dropZValue( QString *errorMsg = nullptr );

    /**
     * Drops the m coordinate of the geometry
     * \return true if success
     * \param errorMsg pointer to QString to receive the error message if any
     */
    bool dropMValue( QString *errorMsg = nullptr );

    /**
     * Swaps the x and y coordinates of the geometry
     * \param errorMsg pointer to QString to receive the error message if any
     */
    void swapXy( QString *errorMsg = nullptr );

    /**
     * Checks if \a geom is valid.
     *
     * If the geometry is invalid, \a errorMsg will be filled with the reported geometry error.
     *
     * \param errorMsg Error message returned by SFGCAL
     * \param flags with Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, specifies whether self-touching holes are permitted.
     *        OGC validity states that self-touching holes are NOT permitted, whilst other
     *        vendor validity checks (e.g. ESRI) permit self-touching holes.
     */
    bool isValid( Qgis::GeometryValidityFlags flags, QString *errorMsg = nullptr ) const;

    /**
     * Checks if \a geom is empty.
     * \param errorMsg Error message returned by SFGCAL
     */
    bool isEmpty( QString *errorMsg = nullptr ) const;

    /**
     * Computes the area of \a geom.
     * \param errorMsg Error message returned by SFGCAL
     */
    double area( QString *errorMsg = nullptr ) const;

    /**
     * Computes the max length of \a geom.
     * \param errorMsg Error message returned by SFGCAL
     */
    double length( QString *errorMsg = nullptr ) const;

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
    std::unique_ptr<QgsSfcgalGeometry> translate( const QgsVector3D &translation, QString *errorMsg = nullptr ) const;

    /**
     * Scale this geometry by vector \a scaleFactor.
     *
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> scale( const QgsVector3D &scaleFactor, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr ) const;

    /**
     * 2D Rotate this geometry around point \a center by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param center rotation center
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate2D( double angle, const QgsPoint &center, QString *errorMsg = nullptr ) const;

    /**
     * 3D Rotate this geometry around axis \a axisVector by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint(), QString *errorMsg = nullptr ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    bool intersects( const QgsAbstractGeometry *otherGeom, QString *errorMsg = nullptr ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    bool intersects( const QgsSfcgalGeometry &otherGeom, QString *errorMsg = nullptr ) const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the intersection results
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsAbstractGeometry *otherGeom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the intersection results
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsSfcgalGeometry &otherGeom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the combination of this and \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the difference results
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsAbstractGeometry *otherGeom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the difference results
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsSfcgalGeometry &otherGeom, QString *errorMsg = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Triangulates this geometry using constraint 2D Delaunay Triangulation (keep Z if defined)
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> triangulate( QString *errorMsg = nullptr ) const;

    /**
     * Calculate the convex hull (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> convexHull( QString *errorMsg = nullptr ) const;

    /**
     * Calculate the envelope (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> envelope( QString *errorMsg = nullptr ) const;

    /**
     * Cover test on 2D or 3D geometries
     * Checks if this covers \a otherGeom.
     * A 3D covers test is conducted when at least one geometry is 3D; otherwise, a 2D covers test is carried out.
     *
     * \param otherGeom second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     */
    bool covers( const QgsSfcgalGeometry &otherGeom, QString *errorMsg = nullptr ) const;

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
    std::unique_ptr<QgsSfcgalGeometry> buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg = nullptr ) const;

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
    std::unique_ptr<QgsSfcgalGeometry> buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg ) const;

    /**
     * Simplifies a geometry using the CGAL algorithm
     *
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> simplify( double tolerance, bool preserveTopology, QString *errorMsg = nullptr ) const;

    /**
     * Calculate an extrusion of the original geometry.
     * If the operation fails, a null pointer is returned.
     *
     * \param extrusion extrusion vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     */
    std::unique_ptr<QgsSfcgalGeometry> extrude( const QgsVector3D &extrusion, QString *errorMsg = nullptr ) const;

  protected:

    /**
     * Clears any cached parameters associated with the geometry, e.g., bounding boxes
     */
    void clearCache() const;

  private:
    sfcgal::shared_geom mSfcgalGeom;
};


#endif // QGSSGCGAL_GEOMETRY_H
#endif
