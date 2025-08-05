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

SIP_IF_MODULE( HAVE_SFCGAL_SIP )

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractgeometry.h"
#include "qgspoint.h"
#include "qgslinestring.h"
#include "qgssfcgalengine.h"

/**
 * Wraps SFCGAL geometry object.
 *
 * \ingroup core
 * \class QgsSfcgalGeometry
 * \brief SfcgalGeometry geometry type.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsSfcgalGeometry
{
  public:
    //! Constructor for an empty SFCGAL geometry geometry.
    QgsSfcgalGeometry();

    /**
     * Constructor with SFCGAL shared ptr.
     *
     * Will copy the shared ptr.
     */
    SIP_SKIP QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom );

    /**
     * Constructor with QgsAbstractGeometry pointer.
     *
     * Will not take ownership.
     *
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom, QString *errorMsg = nullptr );

    /**
     * Constructor with QgsAbstractGeometry reference.
     *
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom, QString *errorMsg = nullptr );

    /**
    * Constructor with QgsGeometry reference.
    *
     * \param errorMsg Error message returned by SFGCAL
     */
    QgsSfcgalGeometry( const QgsGeometry &qgsGeom, QString *errorMsg = nullptr );

    /**
     * Constructor from WKT
     *
     * \param errorMsg Error message returned by SFGCAL
    */
    QgsSfcgalGeometry( const QString &wkt, QString *errorMsg = nullptr );

    /**
     * Copy constructor
     *
     * \param errorMsg Error message returned by SFGCAL
    */
    QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom, QString *errorMsg = nullptr );

    /**
     * Returns the underlying SFCGAL geometry
     * This operation is always fast, as the SFCGAL geometry representation is maintained for the lifetime of the QgsSfcgalGeometry object.
     */
    SIP_SKIP sfcgal::shared_geom sfcgalGeometry() const { return mSfcgalGeom; }

    /**
     * Returns the geometry converted to a QGIS geometry object.
     * This method is slow to call, as it always involves re-conversion of the underlying SFCGAL geometry object.
     */
    std::unique_ptr<QgsAbstractGeometry> asQgisGeometry( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns type of the geometry as a WKB type (point / linestring / polygon etc.)
     * \param errorMsg Error message returned by SFGCAL
     * \return type of the geometry as a WKB type (point / linestring / polygon etc.)
     */
    Qgis::WkbType wkbType( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns type of the geometry as a OGC string in CamelCase
     * \param errorMsg Error message returned by SFGCAL
     * \return type of the geometry as a OGC string in CamelCase
     */
    QString geometryType( QString *errorMsg SIP_OUT = nullptr ) const SIP_HOLDGIL;

    /**
     * Clones the geometry by performing a deep copy
     */
    std::unique_ptr<QgsSfcgalGeometry> clone() const;

    /**
     * Creates a new geometry from a WKB byte pointer
     * \param wkbPtr WKB byte pointer
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry from WKB
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkb( const QgsConstWkbPtr &wkbPtr, QString *errorMsg SIP_OUT = nullptr );

    /**
     * Export the geometry as WKB
     *
     * \param flags argument specifies flags controlling WKB export behavior.
     * \param errorMsg Error message returned by SFGCAL
     * \return WKB data
     */
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags(), QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Creates a new geometry from a WKT string.
     * \param wkt WTK string
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry from WKT
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkt( const QString &wkt, QString *errorMsg SIP_OUT = nullptr );

    /**
     * Export the geometry as WKT
     *
     * \param precision Floating point precision for WKT coordinates. Setting to -1 yields rational number WKT (not decimal) f.e. "Point(1/3, 1/6, 1/4)". Note that this will produce WKT which is not compatible with other QGIS methods or external libraries.
     * \param errorMsg Error message returned by SFGCAL
     * \return WKT data
     */
    QString asWkt( int precision = -1, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \param errorMsg Error message returned by SFGCAL
     * \return boundary for geometry. May be NULLPTR for some geometry types.
     */
    std::unique_ptr<QgsSfcgalGeometry> boundary( QString *errorMsg SIP_OUT = nullptr ) const;

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
     * \return true if this == other geometry modulo \a epsilon distance
     */
    bool fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns the 3D bounding box for the geometry.
     * \param errorMsg Error message returned by SFGCAL
     * \return geometry bbox
     */
    QgsBox3D boundingBox3D( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     * \param errorMsg Error message returned by SFGCAL
     * \return geometry dimension
     */
    int dimension( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Returns the \a geom part count.
     *
     * - POINT, TRIANGLE, LINESTRING: vertex number
     * - POLYGON, SOLID, POLYHEDRALSURFACE, TRIANGULATEDSURFACE: ring or patch or shell number
     * - MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, MULTISOLID, GEOMETRYCOLLECTION: number of geom in collection
     *
     * \param errorMsg pointer to QString to receive the error message if any
     * \return geometry part count
     */
    int partCount( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value (existing Z values remains unchanged).
     * \param zValue z value to use
     * \param errorMsg pointer to QString to receive the error message if any
     * \return true if success
     */
    bool addZValue( double zValue = 0, QString *errorMsg SIP_OUT = nullptr );

    /**
     * Adds a m-dimension to the geometry, initialized to a preset value (existing M values remains unchanged).
     * \param mValue m value to use
     * \param errorMsg pointer to QString to receive the error message if any
     * \return true if success
     */
    bool addMValue( double mValue = 0, QString *errorMsg SIP_OUT = nullptr );

    /**
     * Drops the z coordinate of the geometry
     * \param errorMsg pointer to QString to receive the error message if any
     * \return true if success
     */
    bool dropZValue( QString *errorMsg SIP_OUT = nullptr );

    /**
     * Drops the m coordinate of the geometry
     * \param errorMsg pointer to QString to receive the error message if any
     * \return true if success
     */
    bool dropMValue( QString *errorMsg SIP_OUT = nullptr );

    /**
     * Swaps the x and y coordinates of the geometry
     * \param errorMsg pointer to QString to receive the error message if any
     */
    void swapXy( QString *errorMsg SIP_OUT = nullptr );

    /**
     * Checks if \a geom is valid.
     *
     * If the geometry is invalid, \a errorMsg will be filled with the reported geometry error.
     *
     * \param errorMsg Error message returned by SFGCAL
     * \param flags with Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, specifies whether self-touching holes are permitted.
     *        OGC validity states that self-touching holes are NOT permitted, whilst other
     *        vendor validity checks (e.g. ESRI) permit self-touching holes.
     * \return true if valid
     */
    bool isValid( Qgis::GeometryValidityFlags flags, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Checks if \a geom is empty.
     * \param errorMsg Error message returned by SFGCAL
     * \return true if empty
     */
    bool isEmpty( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Computes the area of \a geom.
     * \param errorMsg Error message returned by SFGCAL
     * \return geometry area
     */
    double area( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Computes the max length of \a geom.
     * \param errorMsg Error message returned by SFGCAL
     * \return geometry length
     */
    double length( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Checks this geometry is simple.
     *
     * \see QgsSfcgalEngine::isSimple
     *
     * \param errorMsg Error message returned by SFGCAL
     * \return true if simple
     */
    bool isSimple( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculates the centroid of this geometry.
     *
     * \param errorMsg Error message returned by SFGCAL
     * \return geometry centroid
     */
    QgsPoint centroid( QString *errorMsg ) const;

    /**
     * Translate this geometry by vector \a translation.
     *
     * \param translation translation vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> translate( const QgsVector3D &translation, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Scale this geometry by vector \a scaleFactor.
     *
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> scale( const QgsVector3D &scaleFactor, const QgsPoint &center = QgsPoint(), QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * 2D Rotate this geometry around point \a center by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param center rotation center
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate2D( double angle, const QgsPoint &center, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * 3D Rotate this geometry around axis \a axisVector by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint(), QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \return true if intersection exists
     */
    bool intersects( const QgsAbstractGeometry *otherGeom, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \return true if intersection exists
     */
    bool intersects( const QgsSfcgalGeometry &otherGeom, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the intersection results
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsAbstractGeometry *otherGeom, QString *errorMsg SIP_OUT = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the intersection results
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsSfcgalGeometry &otherGeom, QString *errorMsg SIP_OUT = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the combination of this and \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the difference results
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsAbstractGeometry *otherGeom, QString *errorMsg SIP_OUT = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \param parameters can be used to specify parameters which control the difference results
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsSfcgalGeometry &otherGeom, QString *errorMsg SIP_OUT = nullptr, const QgsGeometryParameters &parameters = QgsGeometryParameters() ) const;

    /**
     * Triangulates this geometry using constraint 2D Delaunay Triangulation (keep Z if defined)
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> triangulate( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculate the convex hull (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> convexHull( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculate the envelope (bounding box).
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> envelope( QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Cover test on 2D or 3D geometries
     * Checks if this covers \a otherGeom.
     * A 3D covers test is conducted when at least one geometry is 3D; otherwise, a 2D covers test is carried out.
     *
     * \param otherGeom second geometry to perform the operation
     * \param errorMsg Error message returned by SFGCAL
     * \return true if coverage exists
     */
    bool covers( const QgsSfcgalGeometry &otherGeom, QString *errorMsg SIP_OUT = nullptr ) const;

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
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculate a 2D buffer where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * If the operation fails, a null pointer is returned.
     *
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle the type of buffer to compute. Only round is supported.
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg ) const;

    /**
     * Simplifies a geometry using the CGAL algorithm
     *
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> simplify( double tolerance, bool preserveTopology, QString *errorMsg SIP_OUT = nullptr ) const;

    /**
     * Calculate an extrusion of the original geometry.
     * If the operation fails, a null pointer is returned.
     *
     * \param extrusion extrusion vector (2D or 3D)
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     */
    std::unique_ptr<QgsSfcgalGeometry> extrude( const QgsVector3D &extrusion, QString *errorMsg SIP_OUT = nullptr ) const;

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
