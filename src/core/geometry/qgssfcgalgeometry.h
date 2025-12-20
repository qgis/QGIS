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

SIP_IF_MODULE( HAVE_SFCGAL_SIP )

#include "qgsabstractgeometry.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgssfcgalengine.h"

#include <QtGui/qmatrix4x4.h>

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
     * Constructor with SFCGAL shared ptr \a sfcgalGeom.
     *
     * Will copy the shared ptr.
     */
    SIP_SKIP QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom );

    /**
     * Constructor with QgsAbstractGeometry pointer.
     *
     * Will not take ownership.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom ) SIP_THROW( QgsSfcgalException );

    /**
     * Constructor with QgsAbstractGeometry reference.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom ) SIP_THROW( QgsSfcgalException );

    /**
     * Constructor with QgsGeometry reference.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QgsSfcgalGeometry( const QgsGeometry &qgsGeom ) SIP_THROW( QgsSfcgalException );

    /**
     * Constructor from WKT
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
    */
    QgsSfcgalGeometry( const QString &wkt ) SIP_THROW( QgsSfcgalException );

    /**
     * Copy constructor
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
    */
    QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom ) SIP_THROW( QgsSfcgalException );

    /**
     * Copy assignment operator
    */
    QgsSfcgalGeometry &operator=( const QgsSfcgalGeometry &other ) = default;

    /**
     * Returns the underlying SFCGAL geometry
     * This operation is always fast, as the SFCGAL geometry representation is maintained for the lifetime of the QgsSfcgalGeometry object.
     */
    SIP_SKIP sfcgal::shared_geom sfcgalGeometry() const { return mSfcgalGeom; }

    /**
     * Returns the geometry converted to a QGIS geometry object.
     * This method is slow to call, as it always involves re-conversion of the underlying SFCGAL geometry object.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsAbstractGeometry> asQgisGeometry() const SIP_THROW( QgsSfcgalException );

    /**
     * Returns type of the geometry as a WKB type (point / linestring / polygon etc.)
     *
     * \return type of the geometry as a WKB type (point / linestring / polygon etc.)
     *
     * \throws QgsNotSupportedException when working with primitive and SFCGAL is less than 2.3.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    Qgis::WkbType wkbType() const SIP_THROW( QgsSfcgalException );

    /**
     * Returns type of the geometry as a OGC string in CamelCase
     * \return type of the geometry as a OGC string in CamelCase
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException when working with primitive and SFCGAL is less than 2.3.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QString geometryType() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException ) SIP_HOLDGIL;

    /**
     * Clones the geometry by performing a deep copy
     */
    std::unique_ptr<QgsSfcgalGeometry> clone() const;

    /**
     * Creates a new geometry from a WKB byte pointer
     * \param wkbPtr WKB byte pointer
     * \return new geometry from WKB
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkb( const QgsConstWkbPtr &wkbPtr ) SIP_THROW( QgsSfcgalException );

    /**
     * Export the geometry as WKB
     *
     * \param flags argument specifies flags controlling WKB export behavior.
     * \return WKB data
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const SIP_THROW( QgsSfcgalException );

    /**
     * Creates a new geometry from a WKT string.
     * \param wkt WTK string
     * \return new geometry from WKT
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    static std::unique_ptr<QgsSfcgalGeometry> fromWkt( const QString &wkt ) SIP_THROW( QgsSfcgalException );

    /**
     * Export the geometry as WKT
     *
     * \param precision Floating point precision for WKT coordinates. Setting to -1 yields rational number WKT (not decimal) f.e. "Point(1/3, 1/6, 1/4)". Note that this will produce WKT which is not compatible with other QGIS methods or external libraries.
     * \return WKT data
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QString asWkt( int precision = -1 ) const SIP_THROW( QgsSfcgalException );

    /**
     * Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
     * For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
     * \return boundary for geometry. May be NULLPTR for some geometry types.
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> boundary() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Returns true if this == other geometry
     * \param other geometry to perform the operation
     *
     * This operator requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool operator==( const QgsSfcgalGeometry &other ) const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Returns true if this != other geometry
     * \param other geometry to perform the operation
     *
     * This operator requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool operator!=( const QgsSfcgalGeometry &other ) const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Returns true if this == other geometry modulo \a epsilon distance
     * \param other geometry to perform the operation
     * \param epsilon tolerance
     * \return true if this == other geometry modulo \a epsilon distance
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon ) const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
     * 1 for a linestring and 2 for a polygon.
     * \return geometry dimension
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    int dimension() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Returns the \a geom part count.
     *
     * - POINT, TRIANGLE, LINESTRING: vertex number
     * - POLYGON, SOLID, POLYHEDRALSURFACE, TRIANGULATEDSURFACE: ring or patch or shell number
     * - MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, MULTISOLID, GEOMETRYCOLLECTION: number of geom in collection
     *
     * \return geometry part count
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    int partCount() const SIP_THROW( QgsSfcgalException );

    /**
     * Adds a z-dimension to the geometry, initialized to a preset value (existing Z values remains unchanged).
     * \param zValue z value to use
     * \return true if success
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool addZValue( double zValue = 0 ) SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Adds a m-dimension to the geometry, initialized to a preset value (existing M values remains unchanged).
     * \param mValue m value to use
     * \return true if success
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool addMValue( double mValue = 0 ) SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Drops the z coordinate of the geometry
     * \return true if success
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool dropZValue() SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Drops the m coordinate of the geometry
     * \return true if success
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool dropMValue() SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Swaps the x and y coordinates of the geometry
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    void swapXy() SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Checks if \a geom is valid.
     * \return true if valid
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool isValid() const SIP_THROW( QgsSfcgalException );

    /**
     * Checks if \a geom is empty.
     * \return true if empty
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool isEmpty() const SIP_THROW( QgsSfcgalException );

    /**
     * Computes the area of \a geom.
     * \param withDiscretization If true, the area is computed
     * using the real discretization with radial segments. If false, the area is
     * computed for a perfect primitive. Defaults to false.
     * \return geometry area
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    double area( bool withDiscretization = false ) const SIP_THROW( QgsSfcgalException );

    /**
     * Computes the volume of the primitive \a prim.
     * \param withDiscretization If true, the volume is computed
     * using the real discretization with radial segments. If false, the volume is
     * computed for a perfect primitive. Defaults to false.
     * \return primitive volume
     *
     * \pre apply only on primitive
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    double volume( bool withDiscretization = false ) const SIP_THROW( QgsSfcgalException );

    /**
     * Computes the max length of \a geom.
     * \return geometry length
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \pre apply only on geometry
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    double length() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Checks this geometry is simple.
     *
     * \see QgsSfcgalEngine::isSimple
     *
     * \return true if simple
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool isSimple() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Calculates the centroid of this geometry.
     *
     * \return geometry centroid
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    QgsPoint centroid() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Translate this geometry by vector \a translation.
     *
     * \param translation translation vector (2D or 3D)
     * \return new geometry
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> translate( const QgsVector3D &translation ) const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Scale this geometry by vector \a scaleFactor.
     *
     * \param scaleFactor scale factor vector (2D or 3D)
     * \param center optional parameter. If specified, scaling will be performed relative to this center
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> scale( const QgsVector3D &scaleFactor, const QgsPoint &center = QgsPoint() ) const SIP_THROW( QgsSfcgalException );

    /**
     * 2D Rotate this geometry around point \a center by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param center rotation center
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate2D( double angle, const QgsPoint &center ) const SIP_THROW( QgsSfcgalException );

    /**
     * 3D Rotate this geometry around axis \a axisVector by angle \a angle
     *
     * \param angle rotation angle in radians
     * \param axisVector rotation axis
     * \param center optional parameter. If specified, rotation will be applied around axis and center point
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center = QgsPoint() ) const SIP_THROW( QgsSfcgalException );

    /**
     * Apply 3D matrix transform \a mat to geometry \a geom
     *
     * \param mat 4x4 transformation matrix (translation is defined of the 4th column)
     * \param errorMsg Error message returned by SFGCAL
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    std::unique_ptr<QgsSfcgalGeometry> transform( const QMatrix4x4 &mat ) const SIP_THROW( QgsSfcgalException );

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \return true if intersection exists
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool intersects( const QgsAbstractGeometry *otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Checks if \a otherGeom intersects this.
     *
     * \param otherGeom geometry to perform the operation
     * \return true if intersection exists
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool intersects( const QgsSfcgalGeometry &otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsAbstractGeometry *otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the intersection of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> intersection( const QgsSfcgalGeometry &otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the combination of this and \a geomList.
     *
     * \param geomList list of geometries to perform the operation
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> combine( const QVector<QgsAbstractGeometry *> &geomList ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsAbstractGeometry *otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the difference of this and \a otherGeom.
     *
     * \param otherGeom geometry to perform the operation
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> difference( const QgsSfcgalGeometry &otherGeom ) const SIP_THROW( QgsSfcgalException );

    /**
     * Triangulates this geometry using constraint 2D Delaunay Triangulation (keep Z if defined)
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> triangulate() const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the convex hull (bounding box).
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> convexHull() const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate the envelope (bounding box).
     * \return new geometry
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> envelope() const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Cover test on 2D or 3D geometries
     * Checks if this covers \a otherGeom.
     * A 3D covers test is conducted when at least one geometry is 3D; otherwise, a 2D covers test is carried out.
     *
     * \param otherGeom second geometry to perform the operation
     * \return true if coverage exists
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    bool covers( const QgsSfcgalGeometry &otherGeom ) const SIP_THROW( QgsSfcgalException );

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
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D ) const SIP_THROW( QgsSfcgalException );

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
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle ) const SIP_THROW( QgsSfcgalException );

    /**
     * Simplifies a geometry using the CGAL algorithm
     *
     * \param tolerance The distance (in geometry unit) threshold
     * \param preserveTopology Whether to preserve topology during simplification
     * \return new geometry
     *
     * This method requires a QGIS build based on SFCGAL 2.1 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL 2.0.
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> simplify( double tolerance, bool preserveTopology ) const SIP_THROW( QgsNotSupportedException, QgsSfcgalException );

    /**
     * Calculate an extrusion of the original geometry.
     * If the operation fails, a null pointer is returned.
     *
     * \param extrusion extrusion vector (2D or 3D)
     * \return new geometry
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> extrude( const QgsVector3D &extrusion ) const SIP_THROW( QgsSfcgalException );

    /**
     * Calculate a 2D approximate medial axis of \a geom based on its straight skeleton.
     * The approximate medial axis is a simplified representation of a shape’s central skeleton
     * It the geometry is 3D, the approximate medial axis will be calculated from its 2D projection
     * If the operation fails, a null pointer is returned.
     *
     * \return new geometry as 2D multilinestring
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     */
    std::unique_ptr<QgsSfcgalGeometry> approximateMedialAxis() const SIP_THROW( QgsSfcgalException );

    // ============= PRIMITIVE

    /**
     * Constructor with SFCGAL shared ptr \a sfcgalPrim.
     *
     * Will copy the shared ptr.
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    SIP_SKIP QgsSfcgalGeometry( sfcgal::shared_prim sfcgalPrim, sfcgal::primitiveType type );

    /**
     * Create a cube primitive
     * \param size the cube size
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    static std::unique_ptr<QgsSfcgalGeometry> createCube( double size ) SIP_THROW( QgsSfcgalException );

    /**
     * Returns the list of available parameter description for this primitive.
     *
     * Parameter description is a pair of string: name and type. Type can be one of int, double, QgsPoint, QgsVector3D
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    QList<std::pair<QString, QString>> primitiveParameters() const SIP_THROW( QgsSfcgalException );

    /**
     * Returns the parameter value according to its \a name
     *
     * \param name parameter name
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    QVariant primitiveParameter( const QString &name ) const SIP_THROW( QgsSfcgalException );

    /**
     * Updates parameter value
     *
     * \param name parameter name
     * \param value new parameter value
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    void primitiveSetParameter( const QString &name, const QVariant &value ) SIP_THROW( QgsSfcgalException );

    /**
     * Converts the current primitive to geometry. Works only with primitives.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    std::unique_ptr<QgsSfcgalGeometry> primitiveAsPolyhedralSurface() const SIP_THROW( QgsSfcgalException );

    /**
     * Returns the primitive transform matrix.
     *
     * \throws QgsSfcgalException if an error was encountered during the operation
     * \throws QgsNotSupportedException on QGIS builds based on SFCGAL < 2.3.
     */
    QMatrix4x4 primitiveTransform() const SIP_THROW( QgsSfcgalException );

  protected:

    /**
     * Clears any cached parameters associated with the geometry, e.g., bounding boxes
     */
    void clearCache() const;

  private:
    //! \return mSfcgalGeom or geometry obtains from transformed mSfcgalPrim
    sfcgal::shared_geom workingGeom() const;

    sfcgal::shared_geom mSfcgalGeom;
    bool mIsPrimitive = false;

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
    void setPrimitiveTranslate( const QgsVector3D &translation );
    void setPrimitiveScale( const QgsVector3D &scaleFactor, const QgsPoint &center );
    void setPrimitiveRotation( double angle, const QgsVector3D &axisVector, const QgsPoint &center );

    sfcgal::shared_prim mSfcgalPrim;
    sfcgal::primitiveType mPrimType;
    QMatrix4x4 mPrimTransform;
#endif
};


#endif // QGSSGCGAL_GEOMETRY_H
#endif
