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
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom );

    /**
     * Constructor with QgsAbstractGeometry reference.
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom );

    /**
     * Constructor with QgsGeometry reference.
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
     */
    QgsSfcgalGeometry( const QgsGeometry &qgsGeom );

    /**
     * Constructor from WKT
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
    */
    QgsSfcgalGeometry( const QString &wkt );

    /**
     * Copy constructor
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
    */
    QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom );

    /**
     * Returns an error string referring to the last error encountered
     * either when this geometry was created or when an operation
     * was performed on the geometry.
     *
     */
    QString lastError() const SIP_HOLDGIL { return mLastError; }

    /**
     * Returns the underlying SFCGAL geometry
     * This operation is always fast, as the SFCGAL geometry representation is maintained for the lifetime of the QgsSfcgalGeometry object.
     */
    SIP_SKIP sfcgal::shared_geom sfcgalGeometry() const { return mSfcgalGeom; }

    /**
     * Returns the geometry converted to a QGIS geometry object.
     * This method is slow to call, as it always involves re-conversion of the underlying SFCGAL geometry object.
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the geometry.
     */
    std::unique_ptr<QgsAbstractGeometry> asQgisGeometry() const;

    /**
     * Returns type of the geometry as a WKB type (point / linestring / polygon etc.)
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * \return type of the geometry as a WKB type (point / linestring / polygon etc.)
     */
    Qgis::WkbType wkbType() const;

    /**
     * Returns type of the geometry as a OGC string in CamelCase
     * \return type of the geometry as a OGC string in CamelCase
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    QString geometryType() const SIP_THROW( QgsNotSupportedException ) SIP_HOLDGIL;

    /**
     * Clones the geometry by performing a deep copy
     */
    std::unique_ptr<QgsSfcgalGeometry> clone() const;

    /**
     * Creates a new geometry from a WKB byte pointer
     * \param wkbPtr WKB byte pointer
     * \return new geometry from WKB
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkb( const QgsConstWkbPtr &wkbPtr );

    /**
     * Export the geometry as WKB
     *
     * \param flags argument specifies flags controlling WKB export behavior.
     * \return WKB data
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     */
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const;

    /**
     * Creates a new geometry from a WKT string.
     * \param wkt WTK string
     * \return new geometry from WKT
     *
     * If an error was encountered while creating the geometry, more information can be retrieved
     * by calling lastError() on the created geometry.
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkt( const QString &wkt );

    /**
     * Export the geometry as WKT
     *
     * \param precision Floating point precision for WKT coordinates. Setting to -1 yields rational number WKT (not decimal) f.e. "Point(1/3, 1/6, 1/4)". Note that this will produce WKT which is not compatible with other QGIS methods or external libraries.
     * \return WKT data
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     */
    QString asWkt( int precision = -1 ) const;

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \return boundary for geometry. May be NULLPTR for some geometry types.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    std::unique_ptr<QgsSfcgalGeometry> boundary() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns true if this == other geometry
     * \param other geometry to perform the operation
     *
     * This operator requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool operator==( const QgsSfcgalGeometry &other ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns true if this != other geometry
     * \param other geometry to perform the operation
     *
     * This operator requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool operator!=( const QgsSfcgalGeometry &other ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns true if this == other geometry modulo \a epsilon distance
     * \param other geometry to perform the operation
     * \param epsilon tolerance
     * \return true if this == other geometry modulo \a epsilon distance
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     * \return geometry dimension
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    int dimension() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the \a geom part count.
     *
     * - POINT, TRIANGLE, LINESTRING: vertex number
     * - POLYGON, SOLID, POLYHEDRALSURFACE, TRIANGULATEDSURFACE: ring or patch or shell number
     * - MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, MULTISOLID, GEOMETRYCOLLECTION: number of geom in collection
     *
     * \return geometry part count
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     */
    int partCount() const;

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value (existing Z values remains unchanged).
     * \param zValue z value to use
     * \return true if success
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool addZValue( double zValue = 0 ) SIP_THROW( QgsNotSupportedException );

    /**
     * Adds a m-dimension to the geometry, initialized to a preset value (existing M values remains unchanged).
     * \param mValue m value to use
     * \return true if success
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool addMValue( double mValue = 0 ) SIP_THROW( QgsNotSupportedException );

    /**
     * Drops the z coordinate of the geometry
     * \return true if success
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool dropZValue() SIP_THROW( QgsNotSupportedException );

    /**
     * Drops the m coordinate of the geometry
     * \return true if success
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool dropMValue() SIP_THROW( QgsNotSupportedException );

    /**
     * Swaps the x and y coordinates of the geometry
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    void swapXy() SIP_THROW( QgsNotSupportedException );

    /**
     * Checks if \a geom is valid.
     * \return true if valid
     *
     * If the geometry is invalid, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    bool isValid() const;

    /**
     * Checks if \a geom is empty.
     * \return true if empty
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    bool isEmpty() const;

    /**
     * Computes the area of \a geom.
     * \return geometry area
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    double area() const;

    /**
     * Computes the max length of \a geom.
     * \return geometry length
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    double length() const SIP_THROW( QgsNotSupportedException );

    /**
     * Checks this geometry is simple.
     *
     * \see QgsSfcgalEngine::isSimple
     *
     * \return true if simple
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    bool isSimple() const SIP_THROW( QgsNotSupportedException );

    /**
     * Calculates the centroid of this geometry.
     *
     * \return geometry centroid
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    QgsPoint centroid() const SIP_THROW( QgsNotSupportedException );

    /**
     * Translate this geometry by vector \a translation.
     *
     * \param translation translation vector (2D or 3D)
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    std::unique_ptr<QgsSfcgalGeometry> translate( const QgsVector3D &translation ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Scale this geometry by vector \a scaleFactor.
     *
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> scale( const QgsVector3D &scaleFactor, const QgsPoint &center = QgsPoint() ) const;

    /**
     * 2D Rotate this geometry around point \a center by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param center rotation center
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate2D( double angle, const QgsPoint &center ) const;

    /**
     * 3D Rotate this geometry around axis \a axisVector by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint() ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \return true if intersection exists
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    bool intersects( const QgsAbstractGeometry *otherGeom ) const;

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \return true if intersection exists
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    bool intersects( const QgsSfcgalGeometry &otherGeom ) const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsAbstractGeometry *otherGeom )  const;

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsSfcgalGeometry &otherGeom ) const;

    /**
     * Calculate the combination of this and \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> combine( const QVector<QgsAbstractGeometry *> &geomList ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsAbstractGeometry *otherGeom ) const;

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsSfcgalGeometry &otherGeom ) const;

    /**
     * Triangulates this geometry using constraint 2D Delaunay Triangulation (keep Z if defined)
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> triangulate() const;

    /**
     * Calculate the convex hull (bounding box).
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> convexHull() const;

    /**
     * Calculate the envelope (bounding box).
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    std::unique_ptr<QgsSfcgalGeometry> envelope() const SIP_THROW( QgsNotSupportedException );

    /**
     * Cover test on 2D or 3D geometries
     * Checks if this covers \a otherGeom.
     * A 3D covers test is conducted when at least one geometry is 3D; otherwise, a 2D covers test is carried out.
     *
     * \param otherGeom second geometry to perform the operation
     * \return true if coverage exists
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    bool covers( const QgsSfcgalGeometry &otherGeom ) const;

    /**
     * Calculate a 3D buffer where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * It is limited to Point and LineString.
     * If the operation fails, a null pointer is returned.
     *
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle3D the type of buffer to compute
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D ) const;

    /**
     * Calculate a 2D buffer where all points are at \a distance from the original geometry.
     * A negative distance shrinks the geometry rather than expanding it.
     * If the operation fails, a null pointer is returned.
     *
     * \param radius the buffer radius
     * \param segments the number of segments to use for approximating curved
     * \param joinStyle the type of buffer to compute. Only round is supported.
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle ) const;

    /**
     * Simplifies a geometry using the CGAL algorithm
     *
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0 or earlier.
     */
    std::unique_ptr<QgsSfcgalGeometry> simplify( double tolerance, bool preserveTopology ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Calculate an extrusion of the original geometry.
     * If the operation fails, a null pointer is returned.
     *
     * \param extrusion extrusion vector (2D or 3D)
     * \return new geometry
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> extrude( const QgsVector3D &extrusion ) const;

    /**
     * Calculate a 2D approximate medial axis of \a geom based on its straight skeleton.
     * The approximate medial axis is a simplified representation of a shape’s central skeleton
     * It the geometry is 3D, the approximate medial axis will be calculated from its 2D projection
     * If the operation fails, a null pointer is returned.
     *
     * \return new geometry as 2D multilinestring
     *
     * If an error was encountered during the operation, more information can be retrieved
     * by calling lastError() on the geometry.
     *
     */
    std::unique_ptr<QgsSfcgalGeometry> approximateMedialAxis() const;

  protected:

    /**
     * Clears any cached parameters associated with the geometry, e.g., bounding boxes
     */
    void clearCache() const;

  private:
    sfcgal::shared_geom mSfcgalGeom;

    //! Last error encountered
    mutable QString mLastError;
};


#endif // QGSSGCGAL_GEOMETRY_H
#endif
