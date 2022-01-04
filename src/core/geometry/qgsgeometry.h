/***************************************************************************
  qgsgeometry.h - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRY_H
#define QGSGEOMETRY_H

#include <functional>

#include <QDomDocument>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QVector>

#include <climits>
#include <limits>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsabstractgeometry.h"
#include "qgspointxy.h"
#include "qgspoint.h"
#include "qgsfeatureid.h"
#include "qgsvertexid.h"

#ifndef SIP_RUN
#include "json_fwd.hpp"
using namespace nlohmann;
#endif

class QgsGeometryEngine;
class QgsVectorLayer;
class QgsMapToPixel;
class QPainter;
class QgsPolygon;
class QgsLineString;
class QgsCurve;
class QgsFeedback;

/**
 * Polyline as represented as a vector of two-dimensional points.
 *
 * This type has no support for Z/M dimensions and use of QgsPolyline is encouraged instead.
 *
 * \note In QGIS 2.x this type was available as QgsPolyline.
 *
 * \since QGIS 3.0
 */
typedef QVector<QgsPointXY> QgsPolylineXY;

/**
 * Polyline as represented as a vector of points.
 *
 * This type has full support for Z/M dimensions.
 *
 * \since QGIS 3.0
 */
typedef QgsPointSequence QgsPolyline;

//! Polygon: first item of the list is outer ring, inner rings (if any) start from second item
#ifndef SIP_RUN
typedef QVector<QgsPolylineXY> QgsPolygonXY;
#else
typedef QVector<QVector<QgsPointXY>> QgsPolygonXY;
#endif

//! A collection of QgsPoints that share a common collection of attributes
typedef QVector<QgsPointXY> QgsMultiPointXY;

//! A collection of QgsPolylines that share a common collection of attributes
#ifndef SIP_RUN
typedef QVector<QgsPolylineXY> QgsMultiPolylineXY;
#else
typedef QVector<QVector<QgsPointXY>> QgsMultiPolylineXY;
#endif

//! A collection of QgsPolygons that share a common collection of attributes
#ifndef SIP_RUN
typedef QVector<QgsPolygonXY> QgsMultiPolygonXY;
#else
typedef QVector<QVector<QVector<QgsPointXY>>> QgsMultiPolygonXY;
#endif

class QgsRectangle;

class QgsConstWkbPtr;

struct QgsGeometryPrivate;

/**
 * \ingroup core
 * \brief A geometry is the spatial representation of a feature.
 *
 * QgsGeometry acts as a generic container for geometry objects. QgsGeometry objects are implicitly shared,
 * so making copies of geometries is inexpensive. The geometry container class can also be stored inside
 * a QVariant object.
 *
 * The actual geometry representation is stored as a QgsAbstractGeometry within the container, and
 * can be accessed via the get() method or set using the set() method. This gives access to the underlying
 * raw geometry primitive, such as the point, line, polygon, curve or other geometry subclasses.
 *
 * \note QgsGeometry objects are inherently Cartesian/planar geometries. They have no concept of geodesy, and none
 * of the methods or properties exposed from the QgsGeometry API (or QgsAbstractGeometry subclasses) utilize
 * geodesic calculations. Accordingly, properties like length() and area() or spatial operations like buffer()
 * are always calculated using strictly Cartesian mathematics. In contrast, the QgsDistanceArea class exposes
 * methods for working with geodesic calculations and spatial operations on geometries,
 * and should be used whenever calculations which account for the curvature of the Earth (or any other celestial body)
 * are required.
 */
class CORE_EXPORT QgsGeometry
{
    Q_GADGET
    Q_PROPERTY( bool isNull READ isNull )
    Q_PROPERTY( QgsWkbTypes::GeometryType type READ type )

  public:

    //! Constructor
    QgsGeometry() SIP_HOLDGIL;

    //! Copy constructor will prompt a deep copy of the object
    QgsGeometry( const QgsGeometry & );

    /**
     * Creates a deep copy of the object
     * \note not available in Python bindings
     */
    QgsGeometry &operator=( QgsGeometry const &rhs ) SIP_SKIP;

    /**
     * Creates a geometry from an abstract geometry object. Ownership of
     * geom is transferred.
     * \since QGIS 2.10
     */
    explicit QgsGeometry( QgsAbstractGeometry *geom SIP_TRANSFER );

    /**
     * Creates a geometry from an abstract geometry object. Ownership of
     * geom is transferred.
     * \note Not available in Python bindings
     */
    explicit QgsGeometry( std::unique_ptr< QgsAbstractGeometry > geom ) SIP_SKIP;

    virtual ~QgsGeometry();

    /**
     * Returns a non-modifiable (const) reference to the underlying abstract geometry primitive.
     *
     * This is much faster then calling the non-const get() method.
     *
     * \note In QGIS 2.x this method was named geometry().
     *
     * \see set()
     * \see get()
     * \since QGIS 3.0
    */
    const QgsAbstractGeometry *constGet() const SIP_HOLDGIL;

    /**
     * Returns a modifiable (non-const) reference to the underlying abstract geometry primitive.
     *
     * This method can be slow to call, as it may trigger a detachment of the geometry
     * and a deep copy. Where possible, use constGet() instead.
     *
     * \note In QGIS 2.x this method was named geometry().
     *
     * \see constGet()
     * \see set()
     * \since QGIS 3.0
    */
    QgsAbstractGeometry *get();

    /**
     * Sets the underlying geometry store. Ownership of geometry is transferred.
     *
     * \note In QGIS 2.x this method was named setGeometry().
     * \note This method is deprecated for usage in Python and will be removed from Python bindings with QGIS 4.
     *       Using this method will confuse Python's memory management and type information system.
     *       Better create a new QgsGeometry object instead.
     *
     * \see get()
     * \see constGet()
     * \since QGIS 3.0
     */
    void set( QgsAbstractGeometry *geometry SIP_TRANSFER ) SIP_DEPRECATED;

    /**
     * Returns TRUE if the geometry is null (ie, contains no underlying geometry
     * accessible via geometry() ).
     * \see get
     * \see isEmpty()
     * \since QGIS 2.10
     */
    bool isNull() const SIP_HOLDGIL;

    //! Creates a new geometry from a WKT string
    static QgsGeometry fromWkt( const QString &wkt );
    //! Creates a new geometry from a QgsPointXY object
    static QgsGeometry fromPointXY( const QgsPointXY &point ) SIP_HOLDGIL;
    //! Creates a new geometry from a QgsMultiPointXY object
    static QgsGeometry fromMultiPointXY( const QgsMultiPointXY &multipoint );

    /**
     * Creates a new LineString geometry from a list of QgsPointXY points.
     *
     * Using fromPolyline() is preferred, as fromPolyline() is more efficient
     * and will respect any Z or M dimensions present in the input points.
     *
     * \note In QGIS 2.x this method was available as fromPolyline().
     *
     * \see fromPolyline()
     * \since QGIS 3.0
     */
    static QgsGeometry fromPolylineXY( const QgsPolylineXY &polyline );

    /**
     * Creates a new LineString geometry from a list of QgsPoint points.
     *
     * This method will respect any Z or M dimensions present in the input points.
     * E.g. if input points are PointZ type, the resultant linestring will be
     * a LineStringZ type.
     *
     * \since QGIS 3.0
     */
    static QgsGeometry fromPolyline( const QgsPolyline &polyline );

    /**
     * Creates a new geometry from a QgsMultiPolylineXY object.
     */
    static QgsGeometry fromMultiPolylineXY( const QgsMultiPolylineXY &multiline );

#ifndef SIP_RUN

    /**
     * Creates a new geometry from a QgsPolygonXY.
     */
#else

    /**
     * Creates a new polygon geometry from a list of lists of QgsPointXY.
     *
     * The first list of QgsPointXY objects specifies the exterior ring of the polygon, and the remaining
     * lists specify any interior rings.
     *
     * ### Example
     *
     * \code{.py}
     *   # Create a polygon geometry with a single exterior ring (a triangle)
     *   polygon = QgsGeometry.fromPolygonXY([[QgsPointXY(1, 2), QgsPointXY(5, 2), QgsPointXY(5, 10), QgsPointXY(1, 2)]]))
     *
     *   # Create a donut shaped polygon geometry with an interior ring
     *   polygon = QgsGeometry.fromPolygonXY([[QgsPointXY(1, 2), QgsPointXY(5, 2), QgsPointXY(5, 10), QgsPointXY(1, 10), QgsPointXY(1, 2)],
     *                                        [QgsPointXY(3, 4), QgsPointXY(4, 4), QgsPointXY(4, 6), QgsPointXY(3, 6), QgsPointXY(3, 4)]])
     * \endcode
     */
#endif
    static QgsGeometry fromPolygonXY( const QgsPolygonXY &polygon );

    /**
     * Creates a new geometry from a QgsMultiPolygonXY.
     */
    static QgsGeometry fromMultiPolygonXY( const QgsMultiPolygonXY &multipoly );

    //! Creates a new geometry from a QgsRectangle
    static QgsGeometry fromRect( const QgsRectangle &rect ) SIP_HOLDGIL;
    //! Creates a new multipart geometry from a list of QgsGeometry objects
    static QgsGeometry collectGeometry( const QVector<QgsGeometry> &geometries );

    /**
     * Creates a wedge shaped buffer from a \a center point.
     *
     * The \a azimuth gives the angle (in degrees) for the middle of the wedge to point.
     * The buffer width (in degrees) is specified by the \a angularWidth parameter. Note that the
     * wedge will extend to half of the \a angularWidth either side of the \a azimuth direction.
     *
     * The outer radius of the buffer is specified via \a outerRadius, and optionally an
     * \a innerRadius can also be specified.
     *
     * The returned geometry will be a CurvePolygon geometry containing circular strings. It may
     * need to be segmentized to convert to a standard Polygon geometry.
     *
     * \since QGIS 3.2
     */
    static QgsGeometry createWedgeBuffer( const QgsPoint &center, double azimuth, double angularWidth,
                                          double outerRadius, double innerRadius = 0 );

    /**
     * Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
     * This class will take ownership of the buffer.
     * \note not available in Python bindings
     */
    void fromWkb( unsigned char *wkb, int length ) SIP_SKIP;

    /**
     * Set the geometry, feeding in the buffer containing OGC Well-Known Binary
     * \since QGIS 3.0
     */
    void fromWkb( const QByteArray &wkb );

    /**
     * Returns type of the geometry as a WKB type (point / linestring / polygon etc.)
     * \see type
     */
    QgsWkbTypes::Type wkbType() const SIP_HOLDGIL;

    /**
     * Returns type of the geometry as a QgsWkbTypes::GeometryType
     * \see wkbType
     */
    QgsWkbTypes::GeometryType type() const SIP_HOLDGIL;

    /**
     * Returns TRUE if the geometry is empty (eg a linestring with no vertices,
     * or a collection with no geometries). A null geometry will always
     * return TRUE for isEmpty().
     * \see isNull()
     */
    bool isEmpty() const;

    //! Returns TRUE if WKB of the geometry is of WKBMulti* type
    bool isMultipart() const SIP_HOLDGIL;

    /**
     * Test if this geometry is exactly equal to another \a geometry.
     *
     * This is a strict equality check, where the underlying geometries must
     * have exactly the same type, component vertices and vertex order.
     *
     * Calling this method is dramatically faster than the topological
     * equality test performed by isGeosEqual().
     *
     * \note Comparing two null geometries will return FALSE.
     *
     * \see isGeosEqual()
     * \since QGIS 1.5
     */
    bool equals( const QgsGeometry &geometry ) const;

    /**
     * Compares the geometry with another geometry using GEOS.
     *
     * This method performs a slow, topological check, where geometries
     * are considered equal if all of the their component edges overlap. E.g.
     * lines with the same vertex locations but opposite direction will be
     * considered equal by this method.
     *
     * Consider using the much faster, stricter equality test performed
     * by equals() instead.
     *
     * \note Comparing two null geometries will return FALSE.
     *
     * \see equals()
     * \since QGIS 1.5
     */
    bool isGeosEqual( const QgsGeometry & ) const;

    /**
     * Checks validity of the geometry using GEOS.
     *
     * The \a flags parameter indicates optional flags which control the type of validity checking performed.
     *
     * \since QGIS 1.5
     */
    bool isGeosValid( Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const;

    /**
     * Determines whether the geometry is simple (according to OGC definition),
     * i.e. it has no anomalous geometric points, such as self-intersection or self-tangency.
     * Uses GEOS library for the test.
     * \note This is useful mainly for linestrings and linear rings. Polygons are simple by definition,
     * for checking anomalies in polygon geometries one can use isGeosValid().
     * \since QGIS 3.0
     */
    bool isSimple() const;

    /**
     * Returns TRUE if the geometry is a polygon that is almost an axis-parallel rectangle.
     *
     * The \a maximumDeviation argument specifes the maximum angle (in degrees) that the polygon edges
     * are allowed to deviate from axis parallel lines.
     *
     * By default the check will permit polygons with more than 4 edges, so long as the overall shape of
     * the polygon is an axis-parallel rectangle (i.e. it is tolerant to rectangles with additional vertices
     * added along the rectangle sides). If \a simpleRectanglesOnly is set to TRUE then the method will
     * only return TRUE if the geometry is a simple rectangle consisting of 4 edges.
     *
     * \since QGIS 3.20
     */
    bool isAxisParallelRectangle( double maximumDeviation, bool simpleRectanglesOnly = false ) const;

    /**
     * Returns the planar, 2-dimensional area of the geometry.
     *
     * \warning QgsGeometry objects are inherently Cartesian/planar geometries, and the area
     * returned by this method is calculated using strictly Cartesian mathematics. In contrast,
     * the QgsDistanceArea class exposes methods for calculating the areas of geometries using
     * geodesic calculations which account for the curvature of the Earth (or any other
     * celestial body).
     *
     * \see length()
     * \since QGIS 1.5
     */
    double area() const;

    /**
     * Returns the planar, 2-dimensional length of geometry.
     *
     * If the geometry is a polygon geometry then the perimeter of the polygon will be returned.
     *
     * \warning QgsGeometry objects are inherently Cartesian/planar geometries, and the length
     * returned by this method is calculated using strictly Cartesian mathematics. In contrast,
     * the QgsDistanceArea class exposes methods for calculating the lengths of geometries using
     * geodesic calculations which account for the curvature of the Earth (or any other
     * celestial body).
     *
     * \see area()
     * \since QGIS 1.5
     */
    double length() const;

    /**
     * Returns the minimum distance between this geometry and another geometry.
     * Will return a negative value if either geometry is empty or null.
     *
     * \warning QgsGeometry objects are inherently Cartesian/planar geometries, and the distance
     * returned by this method is calculated using strictly Cartesian mathematics.
     */
    double distance( const QgsGeometry &geom ) const;

#ifndef SIP_RUN

    // TODO QGIS 4: consider renaming vertices_begin, vertices_end, parts_begin, parts_end, etc
    // to camelCase

    /**
     * Returns STL-style iterator pointing to the first vertex of the geometry
     * \since QGIS 3.0
     */
    QgsAbstractGeometry::vertex_iterator vertices_begin() const;

    /**
     * Returns STL-style iterator pointing to the imaginary vertex after the last vertex of the geometry
     * \since QGIS 3.0
     */
    QgsAbstractGeometry::vertex_iterator vertices_end() const;
#endif

    /**
     * Returns a read-only, Java-style iterator for traversal of vertices of all the geometry, including all geometry parts and rings.
     *
     * \warning The iterator returns a copy of individual vertices, and accordingly geometries cannot be
     * modified using the iterator. See transformVertices() for a safe method to modify vertices "in-place".
     *
     * ### Example
     *
     * \code{.py}
     *   # print the x and y coordinate for each vertex in a LineString
     *   geometry = QgsGeometry.fromWkt( 'LineString( 0 0, 1 1, 2 2)' )
     *   for v in geometry.vertices():
     *       print(v.x(), v.y())
     *
     *   # vertex iteration includes all parts and rings
     *   geometry = QgsGeometry.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for v in geometry.vertices():
     *       print(v.x(), v.y())
     * \endcode
     *
     * \see parts()
     * \since QGIS 3.0
     */
    QgsVertexIterator vertices() const;

#ifndef SIP_RUN

    /**
     * Returns STL-style iterator pointing to the first part of the geometry.
     *
     * This method forces a detach. Use const_parts_begin() to avoid the detach
     * if the parts are not going to be modified.
     *
     * \since QGIS 3.6
     */
    QgsAbstractGeometry::part_iterator parts_begin();

    /**
     * Returns STL-style iterator pointing to the imaginary part after the last part of the geometry.
     *
     * This method forces a detach. Use const_parts_begin() to avoid the detach
     * if the parts are not going to be modified.
     *
     * \since QGIS 3.6
     */
    QgsAbstractGeometry::part_iterator parts_end();

    /**
     * Returns STL-style const iterator pointing to the first part of the geometry.
     *
     * This method avoids a detach and is more efficient then parts_begin() for read
     * only iteration.
     *
     * \since QGIS 3.6
     */
    QgsAbstractGeometry::const_part_iterator const_parts_begin() const;

    /**
     * Returns STL-style iterator pointing to the imaginary part after the last part of the geometry.
     *
     * This method avoids a detach and is more efficient then parts_end() for read
     * only iteration.
     *
     * \since QGIS 3.6
     */
    QgsAbstractGeometry::const_part_iterator const_parts_end() const;
#endif

    /**
     * Returns Java-style iterator for traversal of parts of the geometry. This iterator
     * can safely be used to modify parts of the geometry.
     *
     * This method forces a detach. Use constParts() to avoid the detach
     * if the parts are not going to be modified.
     *
     * ### Example
     *
     * \code{.py}
     *   # print the WKT representation of each part in a multi-point geometry
     *   geometry = QgsGeometry.fromWkt( 'MultiPoint( 0 0, 1 1, 2 2)' )
     *   for part in geometry.parts():
     *       print(part.asWkt())
     *
     *   # single part geometries only have one part - this loop will iterate once only
     *   geometry = QgsGeometry.fromWkt( 'LineString( 0 0, 10 10 )' )
     *   for part in geometry.parts():
     *       print(part.asWkt())
     *
     *   # parts can be modified during the iteration
     *   geometry = QgsGeometry.fromWkt( 'MultiPoint( 0 0, 1 1, 2 2)' )
     *   for part in geometry.parts():
     *       part.transform(ct)
     *
     *   # part iteration can also be combined with vertex iteration
     *   geometry = QgsGeometry.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for part in geometry.parts():
     *       for v in part.vertices():
     *           print(v.x(), v.y())
     *
     * \endcode
     *
     * \see constParts()
     * \see vertices()
     * \since QGIS 3.6
     */
    QgsGeometryPartIterator parts();

    /**
     * Returns Java-style iterator for traversal of parts of the geometry. This iterator
     * returns read-only references to parts and cannot be used to modify the parts.
     *
     * Unlike parts(), this method does not force a detach and is more efficient if read-only
     * iteration only is required.
     *
     * ### Example
     *
     * \code{.py}
     *   # print the WKT representation of each part in a multi-point geometry
     *   geometry = QgsGeometry.fromWkt( 'MultiPoint( 0 0, 1 1, 2 2)' )
     *   for part in geometry.constParts():
     *       print(part.asWkt())
     *
     *   # single part geometries only have one part - this loop will iterate once only
     *   geometry = QgsGeometry.fromWkt( 'LineString( 0 0, 10 10 )' )
     *   for part in geometry.constParts():
     *       print(part.asWkt())
     *
     *   # part iteration can also be combined with vertex iteration
     *   geometry = QgsGeometry.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for part in geometry.constParts():
     *       for v in part.vertices():
     *           print(v.x(), v.y())
     *
     * \endcode
     *
     * \see parts()
     * \see vertices()
     * \since QGIS 3.6
     */
    QgsGeometryConstPartIterator constParts() const;

    /**
     * Returns the Hausdorff distance between this geometry and \a geom. This is basically a measure of how similar or dissimilar 2 geometries are.
     *
     * This algorithm is an approximation to the standard Hausdorff distance. This approximation is exact or close enough for a large
     * subset of useful cases. Examples of these are:
     *
     * - computing distance between Linestrings that are roughly parallel to each other,
     *   and roughly equal in length. This occurs in matching linear networks.
     * - Testing similarity of geometries.
     *
     * If the default approximate provided by this method is insufficient, use hausdorffDistanceDensify() instead.
     *
     * In case of error -1 will be returned.
     *
     * \see hausdorffDistanceDensify()
     * \since QGIS 3.0
     */
    double hausdorffDistance( const QgsGeometry &geom ) const;

    /**
     * Returns the Hausdorff distance between this geometry and \a geom. This is basically a measure of how similar or dissimilar 2 geometries are.
     *
     * This function accepts a \a densifyFraction argument. The function performs a segment
     * densification before computing the discrete Hausdorff distance. The \a densifyFraction parameter
     * sets the fraction by which to densify each segment. Each segment will be split into a
     * number of equal-length subsegments, whose fraction of the total length is
     * closest to the given fraction.
     *
     * This method can be used when the default approximation provided by hausdorffDistance()
     * is not sufficient. Decreasing the \a densifyFraction parameter will make the
     * distance returned approach the true Hausdorff distance for the geometries.
     *
     * In case of error -1 will be returned.
     *
     * \see hausdorffDistance()
     * \since QGIS 3.0
     */
    double hausdorffDistanceDensify( const QgsGeometry &geom, double densifyFraction ) const;

    /**
     * Returns the Fréchet distance between this geometry and \a geom, restricted to discrete points for both geometries.
     *
     * The Fréchet distance is a measure of similarity between curves that takes into account the location and ordering of the points along the curves.
     * Therefore it is often better than the Hausdorff distance.
     *
     * In case of error -1 will be returned.
     *
     * This method requires a QGIS build based on GEOS 3.7 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.6 or earlier.
     * \see frechetDistanceDensify()
     * \since QGIS 3.20
     */
    double frechetDistance( const QgsGeometry &geom ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the Fréchet distance between this geometry and \a geom, restricted to discrete points for both geometries.
     *
     * The Fréchet distance is a measure of similarity between curves that takes into account the location and ordering of the points along the curves.
     * Therefore it is often better than the Hausdorff distance.
     *
     * This function accepts a \a densifyFraction argument. The function performs a segment
     * densification before computing the discrete Fréchet distance. The \a densifyFraction parameter
     * sets the fraction by which to densify each segment. Each segment will be split into a
     * number of equal-length subsegments, whose fraction of the total length is
     * closest to the given fraction.
     *
     * This method can be used when the default approximation provided by frechetDistance()
     * is not sufficient. Decreasing the \a densifyFraction parameter will make the
     * distance returned approach the true Fréchet distance for the geometries.
     *
     * This method requires a QGIS build based on GEOS 3.7 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.6 or earlier.
     * \see frechetDistance()
     * \since QGIS 3.20
     */
    double frechetDistanceDensify( const QgsGeometry &geom, double densifyFraction ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the vertex closest to the given point, the corresponding vertex index, squared distance snap point / target point
     * and the indices of the vertices before and after the closest vertex.
     * \param point point to search for
     * \param closestVertexIndex will be set to the vertex index of the closest found vertex
     * \param previousVertexIndex will be set to the vertex index of the previous vertex from the closest one. Will be set to -1 if
     * not present.
     * \param nextVertexIndex will be set to the vertex index of the next vertex after the closest one. Will be set to -1 if
     * not present.
     * \param sqrDist will be set to the square distance between the closest vertex and the specified point
     * \returns closest point in geometry. If not found (empty geometry), returns null point and sqrDist is negative.
     */
    QgsPointXY closestVertex( const QgsPointXY &point, int &closestVertexIndex SIP_OUT, int &previousVertexIndex SIP_OUT, int &nextVertexIndex SIP_OUT, double &sqrDist SIP_OUT ) const;

    /**
     * Returns the distance along this geometry from its first vertex to the specified vertex.
     * \param vertex vertex index to calculate distance to
     * \returns distance to vertex (following geometry), or -1 for invalid vertex numbers
     * \warning QgsGeometry objects are inherently Cartesian/planar geometries, and the distance
     * returned by this method is calculated using strictly Cartesian mathematics.
     * \since QGIS 2.16
     */
    double distanceToVertex( int vertex ) const;

    /**
     * Returns the bisector angle for this geometry at the specified vertex.
     * \param vertex vertex index to calculate bisector angle at
     * \returns bisector angle, in radians clockwise from north
     * \see interpolateAngle()
     * \since QGIS 3.0
     */
    double angleAtVertex( int vertex ) const;

    /**
     * Returns the indexes of the vertices before and after the given vertex index.
     *
     * This function takes into account the following factors:
     *
     * # If the given vertex index is at the end of a linestring,
     *    the adjacent index will be -1 (for "no adjacent vertex")
     * # If the given vertex index is at the end of a linear ring
     *    (such as in a polygon), the adjacent index will take into
     *    account the first vertex is equal to the last vertex (and will
     *    skip equal vertex positions).
     */
    void adjacentVertices( int atVertex, int &beforeVertex SIP_OUT, int &afterVertex SIP_OUT ) const;

    /**
     * Insert a new vertex before the given vertex index,
     *  ring and item (first number is index 0)
     *  If the requested vertex number (beforeVertex.back()) is greater
     *  than the last actual vertex on the requested ring and item,
     *  it is assumed that the vertex is to be appended instead of inserted.
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point).
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool insertVertex( double x, double y, int beforeVertex );

    /**
     * Insert a new vertex before the given vertex index,
     *  ring and item (first number is index 0)
     *  If the requested vertex number (beforeVertex.back()) is greater
     *  than the last actual vertex on the requested ring and item,
     *  it is assumed that the vertex is to be appended instead of inserted.
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point).
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool insertVertex( const QgsPoint &point, int beforeVertex );

    /**
     * Moves the vertex at the given position number
     * and item (first number is index 0)
     * to the given coordinates.
     * Returns FALSE if atVertex does not correspond to a valid vertex
     * on this geometry
     */
    bool moveVertex( double x, double y, int atVertex );

    /**
     * Moves the vertex at the given position number
     * and item (first number is index 0)
     * to the given coordinates.
     * Returns FALSE if atVertex does not correspond to a valid vertex
     * on this geometry
     */
    bool moveVertex( const QgsPoint &p, int atVertex );

    /**
     * Deletes the vertex at the given position number and item
     * (first number is index 0)
     * \returns FALSE if atVertex does not correspond to a valid vertex
     * on this geometry (including if this geometry is a Point),
     * or if the number of remaining vertices in the linestring
     * would be less than two.
     * It is up to the caller to distinguish between
     * these error conditions.  (Or maybe we add another method to this
     * object to help make the distinction?)
     */
    bool deleteVertex( int atVertex );

    /**
     * Converts the vertex at the given position from/to circular
     * \returns FALSE if atVertex does not correspond to a valid vertex
     * on this geometry (including if this geometry is a Point),
     * or if the specified vertex can't be converted (e.g. start/end points).
     * \since QGIS 3.20
     */
    bool toggleCircularAtVertex( int atVertex );

    /**
     * Returns coordinates of a vertex.
     * \param atVertex index of the vertex
     * \returns Coordinates of the vertex or empty QgsPoint on error
     */
    QgsPoint vertexAt( int atVertex ) const;

    /**
     * Returns the squared Cartesian distance between the given point
     * to the given vertex index (vertex at the given position number,
     * ring and item (first number is index 0))
     */
    double sqrDistToVertexAt( QgsPointXY &point SIP_IN, int atVertex ) const;

    /**
     * Returns the nearest (closest) point on this geometry to another geometry.
     * \see shortestLine()
     * \since QGIS 2.14
     */
    QgsGeometry nearestPoint( const QgsGeometry &other ) const;

    /**
     * Returns the shortest line joining this geometry to another geometry.
     * \see nearestPoint()
     *
     * \warning QgsGeometry objects are inherently Cartesian/planar geometries, and the line
     * returned by this method is calculated using strictly Cartesian mathematics. See QgsDistanceArea
     * for similar methods which account for the curvature of an ellipsoidal body such as the Earth.
     *
     * \since QGIS 2.14
     */
    QgsGeometry shortestLine( const QgsGeometry &other ) const;

    /**
     * Searches for the closest vertex in this geometry to the given point.
     * \param point Specifiest the point for search
     * \param atVertex Receives index of the closest vertex
     * \returns The squared Cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestVertexWithContext( const QgsPointXY &point, int &atVertex SIP_OUT ) const;

    /**
     * Searches for the closest segment of geometry to the given point
     * \param point Specifies the point for search
     * \param minDistPoint Receives the nearest point on the segment
     * \param nextVertexIndex Receives index of the next vertex after the closest segment. The vertex
     * before the closest segment is always nextVertexIndex - 1
     * \param leftOrRightOfSegment Out: Returns if the point is located on the left or right side of the geometry ( < 0 means left, > 0 means right, 0 indicates
     * that the test was unsuccessful, e.g. for a point exactly on the line)
     * \param epsilon epsilon for segment snapping
     * \returns The squared Cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext( const QgsPointXY &point, QgsPointXY &minDistPoint SIP_OUT, int &nextVertexIndex SIP_OUT, int *leftOrRightOfSegment SIP_OUT = nullptr, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;

    /**
     * Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     * \param ring The ring to be added
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult addRing( const QVector<QgsPointXY> &ring );

    /**
     * Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     * \param ring The ring to be added
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult addRing( QgsCurve *ring SIP_TRANSFER );

    /**
     * Adds a new part to a the geometry.
     * \param points points describing part to add
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult addPart( const QVector<QgsPointXY> &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) SIP_PYNAME( addPointsXY );

    /**
     * Adds a new part to a the geometry.
     * \param points points describing part to add
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult addPart( const QgsPointSequence &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) SIP_PYNAME( addPoints );

    /**
     * Adds a new part to this geometry.
     * \param part part to add (ownership is transferred)
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult addPart( QgsAbstractGeometry *part SIP_TRANSFER, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );

    /**
     * Adds a new island polygon to a multipolygon feature
     * \returns OperationResult a result code: success or reason of failure
     * \note available in python bindings as addPartGeometry
     */
    Qgis::GeometryOperationResult addPart( const QgsGeometry &newPart ) SIP_PYNAME( addPartGeometry );

    /**
     * Removes the interior rings from a (multi)polygon geometry. If the minimumAllowedArea
     * parameter is specified then only rings smaller than this minimum
     * area will be removed.
     * \since QGIS 3.0
     */
    QgsGeometry removeInteriorRings( double minimumAllowedArea = -1 ) const;

    /**
     * Translates this geometry by dx, dy, dz and dm.
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );

    /**
     * Transforms this geometry as described by the coordinate transform \a ct.
     *
     * The transformation defaults to a forward transform, but the direction can be swapped
     * by setting the \a direction argument.
     *
     * By default, z-coordinates are not transformed, even if the coordinate transform
     * includes a vertical datum transformation. To transform z-coordinates, set
     * \a transformZ to TRUE. This requires that the z coordinates in the geometry represent
     * height relative to the vertical datum of the source CRS (generally ellipsoidal heights)
     * and are expressed in its vertical units (generally meters).
     *
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward, bool transformZ = false ) SIP_THROW( QgsCsException );

    /**
     * Transforms the x and y components of the geometry using a QTransform object \a t.
     *
     * Optionally, the geometry's z values can be scaled via \a zScale and translated via \a zTranslate.
     * Similarly, m-values can be scaled via \a mScale and translated via \a mTranslate.
     *
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );

    /**
     * Rotate this geometry around the Z axis
     * \param rotation clockwise rotation in degrees
     * \param center rotation center
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult rotate( double rotation, const QgsPointXY &center );

    /**
     * Splits this geometry according to a given line.
     * \param splitLine the line that splits the geometry
     * \param[out] newGeometries list of new geometries that have been created with the split
     * \param topological TRUE if topological editing is enabled
     * \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
     * \param splitFeature Set to TRUE if you want to split a feature, otherwise set to FALSE to split parts
     * \returns OperationResult a result code: success or reason of failure
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED Qgis::GeometryOperationResult splitGeometry( const QVector<QgsPointXY> &splitLine, QVector<QgsGeometry> &newGeometries SIP_OUT, bool topological, QVector<QgsPointXY> &topologyTestPoints SIP_OUT, bool splitFeature = true ) SIP_DEPRECATED;

    /**
     * Splits this geometry according to a given line.
     * \param splitLine the line that splits the geometry
     * \param[out] newGeometries list of new geometries that have been created with the ``splitLine``. If the geometry is 3D, a linear interpolation of the z value is performed on the geometry at split points, see example.
     * \param topological TRUE if topological editing is enabled
     * \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
     * \param splitFeature Set to TRUE if you want to split a feature, otherwise set to FALSE to split parts
     * fix this bug?
     * \param skipIntersectionTest set to TRUE to skip the potentially expensive initial intersection check. Only set this flag if an intersection
     * test has already been performed by the caller! Not available in Python bindings.
     * \returns OperationResult a result code: success or reason of failure
     *
     * Example:
     *
     * \code{.py}
     *  geometry = QgsGeometry.fromWkt('CompoundCurveZ ((2749546.2003820720128715 1262904.45356595050543547 100, 2749557.82053794478997588 1262920.05570670193992555 200))')
     *  split_line = [QgsPoint(2749544.19, 1262914.79), QgsPoint(2749557.64, 1262897.30)]
     *  result, new_geometries, point_xy = geometry.splitGeometry(split_line, False)
     *  print(geometry.asWkt(2))
     *  > LineStringZ (2749549.12 1262908.38 125.14, 2749557.82 1262920.06 200)
     * \endcode
     */
    Qgis::GeometryOperationResult splitGeometry( const QgsPointSequence &splitLine, QVector<QgsGeometry> &newGeometries SIP_OUT, bool topological, QgsPointSequence &topologyTestPoints SIP_OUT, bool splitFeature = true, bool skipIntersectionTest SIP_PYARGREMOVE = false );

    /**
     * Splits this geometry according to a given curve.
     * \param curve the curve that splits the geometry
     * \param[out] newGeometries list of new geometries that have been created with the ``splitLine``. If the geometry is 3D, a linear interpolation of the z value is performed on the geometry at split points, see example.
     * \param preserveCircular whether if circular strings are preserved after splitting
     * \param topological TRUE if topological editing is enabled
     * \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
     * \param splitFeature Set to TRUE if you want to split a feature, otherwise set to FALSE to split parts
     * \returns OperationResult a result code: success or reason of failure
     * \since QGIS 3.16
     */
    Qgis::GeometryOperationResult splitGeometry( const QgsCurve *curve,  QVector<QgsGeometry> &newGeometries SIP_OUT, bool preserveCircular, bool topological, QgsPointSequence &topologyTestPoints SIP_OUT, bool splitFeature = true );

    /**
     * Replaces a part of this geometry with another line
     * \returns OperationResult a result code: success or reason of failure
     */
    Qgis::GeometryOperationResult reshapeGeometry( const QgsLineString &reshapeLineString );

    /**
     * Changes this geometry such that it does not intersect the other geometry
     * \param other geometry that should not be intersect
     * \note Not available in Python
     */
    int makeDifferenceInPlace( const QgsGeometry &other ) SIP_SKIP;

    /**
     * Returns the geometry formed by modifying this geometry such that it does not
     * intersect the other geometry.
     * \param other geometry that should not be intersect
     * \returns difference geometry, or empty geometry if difference could not be calculated
     * \since QGIS 3.0
     */
    QgsGeometry makeDifference( const QgsGeometry &other ) const;

    /**
     * Returns the bounding box of the geometry.
     * \see orientedMinimumBoundingBox()
     */
    QgsRectangle boundingBox() const;

    /**
     * Returns the oriented minimum bounding box for the geometry, which is the smallest (by area)
     * rotated rectangle which fully encompasses the geometry. The area, angle (clockwise in degrees from North),
     * width and height of the rotated bounding box will also be returned.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \see boundingBox()
     * \since QGIS 3.0
     */
    QgsGeometry orientedMinimumBoundingBox( double &area SIP_OUT, double &angle SIP_OUT, double &width SIP_OUT, double &height SIP_OUT ) const;

    /**
     * Returns the oriented minimum bounding box for the geometry, which is the smallest (by area)
     * rotated rectangle which fully encompasses the geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \since QGIS 3.0
     */
    QgsGeometry orientedMinimumBoundingBox() const SIP_SKIP;

    /**
     * Returns the minimal enclosing circle for the geometry.
     * \param center Center of the minimal enclosing circle returneds
     * \param radius Radius of the minimal enclosing circle returned
     * \param segments Number of segments used to segment geometry. \see QgsEllipse::toPolygon()
     * \returns the minimal enclosing circle as a QGIS geometry
     * \since QGIS 3.0
     */
    QgsGeometry minimalEnclosingCircle( QgsPointXY &center SIP_OUT, double &radius SIP_OUT, unsigned int segments = 36 ) const;

    /**
     * Returns the minimal enclosing circle for the geometry.
     * \param segments Number of segments used to segment geometry. \see QgsEllipse::toPolygon()
     * \since QGIS 3.0
     */
    QgsGeometry minimalEnclosingCircle( unsigned int segments = 36 ) const SIP_SKIP;

    /**
     * Attempts to orthogonalize a line or polygon geometry by shifting vertices to make the geometries
     * angles either right angles or flat lines. This is an iterative algorithm which will loop until
     * either the vertices are within a specified tolerance of right angles or a set number of maximum
     * iterations is reached. The angle threshold parameter specifies how close to a right angle or
     * straight line an angle must be before it is attempted to be straightened.
     * \since QGIS 3.0
     */
    QgsGeometry orthogonalize( double tolerance = 1.0E-8, int maxIterations = 1000, double angleThreshold = 15.0 ) const;

    /**
     * Constructs triangular waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of triangular waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see triangularWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry triangularWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized triangular waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual triangular waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the triangular components. Randomized amplitude values will be calculated
     * individually for triangles placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see triangularWaves()
     * \since QGIS 3.24
     */
    QgsGeometry triangularWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;

    /**
     * Constructs square waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of square waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see squareWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry squareWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized square waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual square waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the square components. Randomized amplitude values will be calculated
     * individually for squares placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see squareWaves()
     * \since QGIS 3.24
     */
    QgsGeometry squareWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;

    /**
     * Constructs rounded (sine-like) waves along the boundary of the geometry, with the
     * specified \a wavelength and \a amplitude.
     *
     * By default the \a wavelength argument is treated as a "maximum wavelength", where the actual
     * wavelength will be dynamically adjusted so that an exact number of waves are created
     * along the boundaries of the geometry. If \a strictWavelength is set to TRUE then the \a wavelength
     * will be used exactly and an incomplete pattern may be used for the final waveform.
     *
     * \see roundWavesRandomized()
     * \since QGIS 3.24
     */
    QgsGeometry roundWaves( double wavelength, double amplitude, bool strictWavelength = false ) const;

    /**
     * Constructs randomized rounded (sine-like) waves along the boundary of the geometry, with the
     * specified wavelength and amplitude ranges.
     *
     * The \a minimumWavelength and \a maximumWavelength arguments set the range for the randomized
     * wavelength. This is evaluated for each individual waveform created along the geometry
     * boundaries, so the resultant geometry will consist of many different wavelengths.
     *
     * Similarly, the \a minimumAmplitude and \a maximumAmplitude arguments define the range for the
     * randomized amplitude of the square components. Randomized amplitude values will be calculated
     * individually for waves placed on each either side of the input geometry boundaries.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * \see squareWaves()
     * \since QGIS 3.24
     */
    QgsGeometry roundWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed = 0 ) const;

    /**
     * Applies a dash pattern to a geometry, returning a MultiLineString geometry which is the
     * input geometry stroked along each line/ring with the specified \a pattern.
     *
     * The \a startRule and \a endRule options can be set to control how the dash pattern is adjusted
     * at line endings. If a \a startRule or \a endRule is set, the \a adjustment option defines whether
     * both dash and gaps, or only dash or gap sizes are adjusted to apply the rules.
     *
     * The \a patternOffset option specifies how far along the pattern the result should start at.
     * The offset is applied AFTER any start/end rules are applied.
     *
     * \since QGIS 3.24
     */
    QgsGeometry applyDashPattern( const QVector< double > &pattern,
                                  Qgis::DashPatternLineEndingRule startRule = Qgis::DashPatternLineEndingRule::NoRule,
                                  Qgis::DashPatternLineEndingRule endRule = Qgis::DashPatternLineEndingRule::NoRule,
                                  Qgis::DashPatternSizeAdjustment adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap,
                                  double patternOffset = 0 ) const;

    /**
     * Returns a new geometry with all points or vertices snapped to the closest point of the grid.
     *
     * If the gridified geometry could not be calculated (or was totally collapsed) an empty geometry will be returned.
     * Note that snapping to grid may generate an invalid geometry in some corner cases.
     * It can also be thought as rounding the edges and it may be useful for removing errors.
     * \param hSpacing Horizontal spacing of the grid (x axis). 0 to disable.
     * \param vSpacing Vertical spacing of the grid (y axis). 0 to disable.
     * \param dSpacing Depth spacing of the grid (z axis). 0 (default) to disable.
     * \param mSpacing Custom dimension spacing of the grid (m axis). 0 (default) to disable.
     * \since 3.0
     */
    QgsGeometry snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const;

    /**
     * Removes duplicate nodes from the geometry, wherever removing the nodes does not result in a
     * degenerate geometry.
     *
     * The \a epsilon parameter specifies the tolerance for coordinates when determining that
     * vertices are identical.
     *
     * By default, z values are not considered when detecting duplicate nodes. E.g. two nodes
     * with the same x and y coordinate but different z values will still be considered
     * duplicate and one will be removed. If \a useZValues is TRUE, then the z values are
     * also tested and nodes with the same x and y but different z will be maintained.
     *
     * Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g.
     * a multipoint geometry with overlapping points will not be changed by this method.
     *
     * The function will return TRUE if nodes were removed, or FALSE if no duplicate nodes
     * were found.
     *
     * \since QGIS 3.0
     */
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false );

    /**
     * Returns TRUE if this geometry exactly intersects with a \a rectangle. This test is exact
     * and can be slow for complex geometries.
     *
     * The GEOS library is used to perform the intersection test. Geometries which are not
     * valid may return incorrect results.
     *
     * \see boundingBoxIntersects()
     */
    bool intersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns TRUE if this geometry exactly intersects with another \a geometry. This test is exact
     * and can be slow for complex geometries.
     *
     * The GEOS library is used to perform the intersection test. Geometries which are not
     * valid may return incorrect results.
     *
     * \note For performance critical code, or when testing for intersection against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling intersects() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \see boundingBoxIntersects()
     */
    bool intersects( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the bounding box of this geometry intersects with a \a rectangle. Since this
     * test only considers the bounding box of the geometry, is is very fast to calculate and handles invalid
     * geometries.
     *
     * \see intersects()
     *
     * \since QGIS 3.0
     */
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns TRUE if the bounding box of this geometry intersects with the bounding box of another \a geometry. Since this
     * test only considers the bounding box of the geometries, is is very fast to calculate and handles invalid
     * geometries.
     *
     * \see intersects()
     *
     * \since QGIS 3.0
     */
    bool boundingBoxIntersects( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry contains the point \a p.
     */
    bool contains( const QgsPointXY *p ) const;

    /**
     * Returns TRUE if the geometry completely contains another \a geometry.
     *
     * \note For performance critical code, or when testing for contains against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling contains() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool contains( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry is disjoint of another \a geometry.
     *
     * \note For performance critical code, or when testing for disjoint against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling disjoint() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool disjoint( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry touches another \a geometry.
     *
     * \note For performance critical code, or when testing for touches against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling touches() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool touches( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry overlaps another \a geometry.
     *
     * \note For performance critical code, or when testing for overlaps against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling overlaps() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool overlaps( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry is completely within another \a geometry.
     *
     * \note For performance critical code, or when testing for within against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling within() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool within( const QgsGeometry &geometry ) const;

    /**
     * Returns TRUE if the geometry crosses another \a geometry.
     *
     * \note For performance critical code, or when testing for crosses against many different
     * geometries, consider using QgsGeometryEngine instead. This approach can be many orders of magnitude
     * faster than calling crosses() directly. See createGeometryEngine() for details on how to use the
     * QgsGeometryEngine class.
     *
     * \since QGIS 1.5
     */
    bool crosses( const QgsGeometry &geometry ) const;

    /**
     * Returns a buffer region around this geometry having the given width and with a specified number
     * of segments used to approximate curves
     *
     * \see singleSidedBuffer()
     * \see taperedBuffer()
     */
    QgsGeometry buffer( double distance, int segments ) const;

    /**
     * Returns a buffer region around the geometry, with additional style options.
     * \param distance    buffer distance
     * \param segments    for round joins, number of segments to approximate quarter-circle
     * \param endCapStyle end cap style
     * \param joinStyle   join style for corners in geometry
     * \param miterLimit  limit on the miter ratio used for very sharp corners (JoinStyleMiter only)
     *
     * \see singleSidedBuffer()
     * \see taperedBuffer()
     * \since QGIS 2.4
     */
    QgsGeometry buffer( double distance, int segments, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit ) const;

    /**
     * Returns an offset line at a given distance and side from an input line.
     * \param distance    buffer distance
     * \param segments    for round joins, number of segments to approximate quarter-circle
     * \param joinStyle   join style for corners in geometry
     * \param miterLimit  limit on the miter ratio used for very sharp corners (JoinStyleMiter only)
     * \since QGIS 2.4
     */
    QgsGeometry offsetCurve( double distance, int segments, Qgis::JoinStyle joinStyle, double miterLimit ) const;

    /**
     * Returns a single sided buffer for a (multi)line geometry. The buffer is only
     * applied to one side of the line.
     * \param distance buffer distance
     * \param segments for round joins, number of segments to approximate quarter-circle
     * \param side side of geometry to buffer
     * \param joinStyle join style for corners
     * \param miterLimit limit on the miter ratio used for very sharp corners
     * \returns buffered geometry, or an empty geometry if buffer could not be
     * calculated
     *
     * \see buffer()
     * \see taperedBuffer()
     * \since QGIS 3.0
     */
    QgsGeometry singleSidedBuffer( double distance, int segments, Qgis::BufferSide side,
                                   Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round,
                                   double miterLimit = 2.0 ) const;

    /**
     * Calculates a variable width buffer ("tapered buffer") for a (multi)curve geometry.
     *
     * The buffer begins at a width of \a startWidth at the start of each curve, and
     * ends at a width of \a endWidth. Note that unlike buffer() methods, \a startWidth
     * and \a endWidth are the diameter of the buffer at these points, not the radius.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)curve input geometries will return a null output geometry.
     *
     * \see buffer()
     * \see singleSidedBuffer()
     * \see variableWidthBufferByM()
     * \since QGIS 3.2
     */
    QgsGeometry taperedBuffer( double startWidth, double endWidth, int segments ) const;

    /**
     * Calculates a variable width buffer for a (multi)linestring geometry, where
     * the width at each node is taken from the linestring m values.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)linestring input geometries will return a null output geometry.
     *
     * \see buffer()
     * \see singleSidedBuffer()
     * \see taperedBuffer()
     * \since QGIS 3.2
     */
    QgsGeometry variableWidthBufferByM( int segments ) const;

    /**
     * Extends a (multi)line geometry by extrapolating out the start or end of the line
     * by a specified distance. Lines are extended using the bearing of the first or last
     * segment in the line.
     * \since QGIS 3.0
     */
    QgsGeometry extendLine( double startDistance, double endDistance ) const;

    //! Returns a simplified version of this geometry using a specified tolerance value
    QgsGeometry simplify( double tolerance ) const;

    /**
     * Returns a copy of the geometry which has been densified by adding the specified
     * number of extra nodes within each segment of the geometry.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \see densifyByDistance()
     * \since QGIS 3.0
     */
    QgsGeometry densifyByCount( int extraNodesPerSegment ) const;

    /**
     * Densifies the geometry by adding regularly placed extra nodes inside each segment
     * so that the maximum distance between any two nodes does not exceed the
     * specified \a distance.
     * E.g. specifying a distance 3 would cause the segment [0 0] -> [10 0]
     * to be converted to [0 0] -> [2.5 0] -> [5 0] -> [7.5 0] -> [10 0], since
     * 3 extra nodes are required on the segment and spacing these at 2.5 increments
     * allows them to be evenly spaced over the segment.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \see densifyByCount()
     * \since QGIS 3.0
     */
    QgsGeometry densifyByDistance( double distance ) const;

    /**
     * Attempts to convert a non-curved geometry into a curved geometry type (e.g.
     * LineString to CompoundCurve, Polygon to CurvePolygon).
     *
     * The \a distanceTolerance specifies the maximum deviation allowed between the original location
     * of vertices and where they would fall on the candidate curved geometry.
     *
     * This method only consider a segments as suitable for replacing with an arc if the points are all
     * regularly spaced on the candidate arc. The \a pointSpacingAngleTolerance parameter specifies the maximum
     * angular deviation (in radians) allowed when testing for regular point spacing.
     *
     * \note The API is considered EXPERIMENTAL and can be changed without a notice
     *
     * \since QGIS 3.14
     */
    QgsGeometry convertToCurves( double distanceTolerance = 1e-8, double angleTolerance = 1e-8 ) const;

    /**
     * Returns the center of mass of a geometry.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \note for line based geometries, the center point of the line is returned,
     * and for point based geometries, the point itself is returned
     * \see pointOnSurface()
     * \see poleOfInaccessibility()
     */
    QgsGeometry centroid() const;

    /**
     * Returns a point guaranteed to lie on the surface of a geometry. While the centroid()
     * of a geometry may be located outside of the geometry itself (e.g., for concave shapes),
     * the point on surface will always be inside the geometry.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \see centroid()
     * \see poleOfInaccessibility()
     */
    QgsGeometry pointOnSurface() const;

    /**
     * Calculates the approximate pole of inaccessibility for a surface, which is the
     * most distant internal point from the boundary of the surface. This function
     * uses the 'polylabel' algorithm (Vladimir Agafonkin, 2016), which is an iterative
     * approach guaranteed to find the true pole of inaccessibility within a specified
     * tolerance. More precise tolerances require more iterations and will take longer
     * to calculate.
     * Optionally, the distance to the polygon boundary from the pole can be stored.
     * \see centroid()
     * \see pointOnSurface()
     * \since QGIS 3.0
     */
    QgsGeometry poleOfInaccessibility( double precision, double *distanceToBoundary SIP_OUT = nullptr ) const;

    /**
     * Constructs the Largest Empty Circle for a set of obstacle geometries, up to a
     * specified tolerance.
     *
     * The Largest Empty Circle is the largest circle which has its center in the convex hull of the
     * obstacles (the boundary), and whose interior does not intersect with any obstacle.
     * The circle center is the point in the interior of the boundary which has the farthest distance from
     * the obstacles (up to tolerance). The circle is determined by the center point and a point lying on an
     * obstacle indicating the circle radius.
     * The implementation uses a successive-approximation technique over a grid of square cells covering the obstacles and boundary.
     * The grid is refined using a branch-and-bound algorithm.  Point containment and distance are computed in a performant
     * way by using spatial indexes.
     * Returns a two-point linestring, with one point at the center of the inscribed circle and the other
     * on the boundary of the inscribed circle.
     *
     * This method requires QGIS builds based on GEOS 3.9 or later.
     *
     * \warning the \a tolerance value must be a value greater than 0, or the algorithm may never converge on a solution
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.8 or earlier.
     *
     * \since QGIS 3.20
     */
    QgsGeometry largestEmptyCircle( double tolerance, const QgsGeometry &boundary = QgsGeometry() ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns a linestring geometry which represents the minimum diameter of the geometry.
     *
     * The minimum diameter is defined to be the width of the smallest band that
     * contains the geometry, where a band is a strip of the plane defined
     * by two parallel lines. This can be thought of as the smallest hole that the geometry
     * can be moved through, with a single rotation.
     *
     * This method requires a QGIS build based on GEOS 3.6 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.5 or earlier.
     *
     * \since QGIS 3.20
     */
    QgsGeometry minimumWidth() const SIP_THROW( QgsNotSupportedException );

    /**
     * Computes the minimum clearance of a geometry.
     *
     * The minimum clearance is the smallest amount by which
     * a vertex could be moved to produce an invalid polygon, a non-simple linestring, or a multipoint with
     * repeated points.  If a geometry has a minimum clearance of 'eps', it can be said that:
     *
     * - No two distinct vertices in the geometry are separated by less than 'eps'
     * - No vertex is closer than 'eps' to a line segment of which it is not an endpoint.
     *
     * If the minimum clearance cannot be defined for a geometry (such as with a single point, or a multipoint
     * whose points are identical) a value of infinity will be returned.
     *
     * If an error occurs while calculating the clearance NaN will be returned.
     *
     * This method requires a QGIS build based on GEOS 3.6 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.5 or earlier.
     *
     * \since QGIS 3.20
     */
    double minimumClearance() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns a LineString whose endpoints define the minimum clearance of a geometry.
     *
     * If the geometry has no minimum clearance, an empty LineString will be returned.
     *
     * This method requires a QGIS build based on GEOS 3.6 or later.
     *
     * \throws QgsNotSupportedException on QGIS builds based on GEOS 3.5 or earlier.
     *
     * \since QGIS 3.20
     */
    QgsGeometry minimumClearanceLine() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the smallest convex polygon that contains all the points in the geometry.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     */
    QgsGeometry convexHull() const;

    /**
     * Creates a Voronoi diagram for the nodes contained within the geometry.
     *
     * Returns the Voronoi polygons for the nodes contained within the geometry.
     * If \a extent is specified then it will be used as a clipping envelope for the diagram.
     * If no extent is set then the clipping envelope will be automatically calculated.
     * In either case the diagram will be clipped to the larger of the provided envelope
     * OR the envelope surrounding all input nodes.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the diagram calculation.
     * If \a edgesOnly is TRUE than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry voronoiDiagram( const QgsGeometry &extent = QgsGeometry(), double tolerance = 0.0, bool edgesOnly = false ) const;

    /**
     * Returns the Delaunay triangulation for the vertices of the geometry.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the triangulation.
     * If \a edgesOnly is TRUE than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry delaunayTriangulation( double tolerance = 0.0, bool edgesOnly = false ) const;

    /**
     * Returns a (Multi)LineString representing the fully noded version of a collection of linestrings.
     *
     * The noding preserves all of the input nodes, and introduces the least possible number of new nodes.
     * The resulting linework is dissolved (duplicate lines are removed).
     *
     * The input geometry type should be a (Multi)LineString.
     *
     * \since QGIS 3.20
     */
    QgsGeometry node() const;

    /**
     * Find paths shared between the two given lineal geometries (this and \a other).
     *
     * Returns a GeometryCollection having two elements:
     *
     * - first element is a MultiLineString containing shared paths
     *   having the same direction on both inputs
     * - second element is a MultiLineString containing shared paths
     *   having the opposite direction on the two inputs
     *
     * Returns a null geometry on exception.
     *
     * \since QGIS 3.20
     */
    QgsGeometry sharedPaths( const QgsGeometry &other ) const;

    /**
     * Subdivides the geometry. The returned geometry will be a collection containing subdivided parts
     * from the original geometry, where no part has more then the specified maximum number of nodes (\a maxNodes).
     *
     * This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially
     * indexed and faster to perform further operations such as intersects on. The returned geometry parts may
     * not be valid and may contain self-intersections.
     *
     * The minimum allowed value for \a maxNodes is 8.
     *
     * Curved geometries will be segmentized before subdivision.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \since QGIS 3.0
     */
    QgsGeometry subdivide( int maxNodes = 256 ) const;

    /**
     * Returns an interpolated point on the geometry at the specified \a distance.
     *
     * If the original geometry is a polygon type, the boundary of the polygon
     * will be used during interpolation. If the original geometry is a point
     * type, a null geometry will be returned.
     *
     * If z or m values are present, the output z and m will be interpolated using
     * the existing vertices' z or m values.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * \see lineLocatePoint()
     * \since QGIS 2.0
     */
    QgsGeometry interpolate( double distance ) const;

    /**
     * Returns a distance representing the location along this linestring of the closest point
     * on this linestring geometry to the specified point. Ie, the returned value indicates
     * how far along this linestring you need to traverse to get to the closest location
     * where this linestring comes to the specified point.
     * \param point point to seek proximity to
     * \returns distance along line, or -1 on error
     * \note only valid for linestring geometries
     * \see interpolate()
     * \since QGIS 3.0
     */
    double lineLocatePoint( const QgsGeometry &point ) const;

    /**
     * Returns the angle parallel to the linestring or polygon boundary at the specified distance
     * along the geometry. Angles are in radians, clockwise from north.
     * If the distance coincides precisely at a node then the average angle from the segment either side
     * of the node is returned.
     * \param distance distance along geometry
     * \see angleAtVertex()
     * \since QGIS 3.0
     */
    double interpolateAngle( double distance ) const;

    /**
     * Returns a geometry representing the points shared by this geometry and other.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     */
    QgsGeometry intersection( const QgsGeometry &geometry ) const;

    /**
     * Clips the geometry using the specified \a rectangle.
     *
     * Performs a fast, non-robust intersection between the geometry and
     * a \a rectangle. The returned geometry may be invalid.
     * \since QGIS 3.0
     */
    QgsGeometry clipped( const QgsRectangle &rectangle );

    /**
     * Returns a geometry representing all the points in this geometry and other (a
     * union geometry operation).
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \note this operation is not called union since its a reserved word in C++.
     */
    QgsGeometry combine( const QgsGeometry &geometry ) const;

    /**
     * Merges any connected lines in a LineString/MultiLineString geometry and
     * converts them to single line strings.
     * \returns a LineString or MultiLineString geometry, with any connected lines
     * joined. An empty geometry will be returned if the input geometry was not a
     * MultiLineString geometry.
     * \since QGIS 3.0
     */
    QgsGeometry mergeLines() const;

    /**
     * Returns a geometry representing the points making up this geometry that do not make up other.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     */
    QgsGeometry difference( const QgsGeometry &geometry ) const;

    /**
     * Returns a geometry representing the points making up this geometry that do not make up other.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling lastError() on the returned geometry.
     */
    QgsGeometry symDifference( const QgsGeometry &geometry ) const;

    //! Returns an extruded version of this geometry.
    QgsGeometry extrude( double x, double y );

#ifndef SIP_RUN

    /**
     * Returns a list of \a count random points generated inside a (multi)polygon geometry
     * (if \a acceptPoint is specified, and restrictive, the number of points returned may
     * be less than \a count).
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * If the source geometry is not a (multi)polygon, an empty list will be returned.
     *
     * The optional \a feedback argument can be used to provide cancellation support during
     * the point generation.
     *
     * The \a acceptPoint function is used to filter result candidates. If the function returns
     * FALSE, then the point will not be accepted and another candidate generated.
     *
     * When \a acceptPoint is specified, \a maxTriesPerPoint defines how many attempts to make
     * before giving up generating a point.
     *
     * \since QGIS 3.10
     */
    QVector< QgsPointXY > randomPointsInPolygon( int count, const std::function< bool( const QgsPointXY & ) > &acceptPoint, unsigned long seed = 0, QgsFeedback *feedback = nullptr, int maxTriesPerPoint = 0 ) const;

    /**
     * Returns a list of \a count random points generated inside a (multi)polygon geometry.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * If the source geometry is not a (multi)polygon, an empty list will be returned.
     *
     * The optional \a feedback argument can be used to provide cancellation support during
     * the point generation.
     *
     * \since QGIS 3.10
     */
    QVector< QgsPointXY > randomPointsInPolygon( int count, unsigned long seed = 0, QgsFeedback *feedback = nullptr ) const;
    ///@cond PRIVATE
#else

    /**
     * Returns a list of \a count random points generated inside a (multi)polygon geometry.
     *
     * Optionally, a specific random \a seed can be used when generating points. If \a seed
     * is 0, then a completely random sequence of points will be generated.
     *
     * This method works only with (multi)polygon geometry types.
     *
     * \throws TypeError if the geometry is not a polygon type
     * \throws ValueError if the geometry is null
     *
     * \since QGIS 3.10
     */
    SIP_PYOBJECT randomPointsInPolygon( int count, unsigned long seed = 0 ) const SIP_TYPEHINT( QgsPolylineXY );
    % MethodCode
    const QgsWkbTypes::GeometryType type = sipCpp->type();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Cannot generate points inside a null geometry." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( type != QgsWkbTypes::PolygonGeometry )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "Cannot generate points inside a %1 geometry. Only Polygon types are permitted." ).arg( QgsWkbTypes::displayString( sipCpp->wkbType() ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector<QgsPointXY>" );
      sipRes = sipConvertFromNewType( new QVector< QgsPointXY >( sipCpp->randomPointsInPolygon( a0, a1 ) ), qvector_type, Py_None );
    }
    % End


#endif
///@endcond

    /**
     * Returns the length of the QByteArray returned by asWkb()
     *
     * The optional \a flags argument specifies flags controlling WKB export behavior
     *
     * \since QGIS 3.16
     */
    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const;

    /**
     * Export the geometry to WKB
     *
     * The optional \a flags argument specifies flags controlling WKB export behavior (since QGIS 3.14).
     *
     * \since QGIS 3.0
     */
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const;

    /**
     * Exports the geometry to WKT
     * \returns TRUE in case of success and FALSE else
     * \note precision parameter added in QGIS 2.4
     */
    QString asWkt( int precision = 17 ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str;
    if ( sipCpp->isNull() )
      str = QStringLiteral( "<QgsGeometry: null>" );
    else
    {
      QString wkt = sipCpp->asWkt();
      if ( wkt.length() > 1000 )
        wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
      str = QStringLiteral( "<QgsGeometry: %1>" ).arg( wkt );
    }
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Exports the geometry to a GeoJSON string.
     */
    QString asJson( int precision = 17 ) const;

    /**
     * Exports the geometry to a json object.
     * \note not available in Python bindings
     * \since QGIS 3.8
     */
    virtual json asJsonObject( int precision = 17 ) const SIP_SKIP;

    /**
     * Attempts to coerce this geometry into the specified destination \a type.
     *
     * This method will do anything possible to force the current geometry into the specified type. E.g.
     *
     * - lines or polygons will be converted to points by return either a single multipoint geometry or multiple
     *   single point geometries.
     * - polygons will be converted to lines by extracting their exterior and interior rings, returning
     *   either a multilinestring or multiple single line strings as dictated by \a type.
     * - lines will be converted to polygon rings if \a type is a polygon type
     * - curved geometries will be segmented if \a type is non-curved.
     * - multi geometries will be converted to a list of single geometries
     * - single geometries will be upgraded to multi geometries
     * - z or m values will be added or dropped as required.
     *
     * Since QGIS 3.24, the parameters \a defaultZ and \a defaultM control the dimension value added when promoting geometries
     * to Z, M or ZM versions.
     * By default 0.0 is used for Z and M.
     *
     * \note This method is much stricter than convertToType(), as it considers the exact WKB type
     * of geometries instead of the geometry family (point/line/polygon), and tries more exhaustively
     * to coerce geometries to the desired \a type. It also correctly maintains curves and z/m values
     * wherever appropriate.
     *
     * \since QGIS 3.14
     */
    QVector< QgsGeometry > coerceToType( QgsWkbTypes::Type type, double defaultZ = 0, double defaultM = 0 ) const;

    /**
     * Try to convert the geometry to the requested type
     * \param destType the geometry type to be converted to
     * \param destMultipart determines if the output geometry will be multipart or not
     * \returns the converted geometry or NULLPTR if the conversion fails.
     *
     * \note The coerceToType() method applies much stricter and more exhaustive attempts to convert
     * between geometry types, and is recommended instead of this method. This method force drops
     * curves and any z or m values present in the geometry.
     *
     * \since QGIS 2.2
     */
    QgsGeometry convertToType( QgsWkbTypes::GeometryType destType, bool destMultipart = false ) const;

    /* Accessor functions for getting geometry data */

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a 2-dimensional point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * \warning If the geometry is not a single-point type (or a multipoint containing a single point)
     * an empty QgsPointXY() will be returned.
     */
    QgsPointXY asPoint() const;
#else

    /**
     * Returns the contents of the geometry as a 2-dimensional point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * This method works only with single-point geometry types.
     *
     * \throws TypeError if the geometry is not a single-point type (or a multipoint containing a single point)
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asPoint() const SIP_TYPEHINT( QgsPointXY );
    % MethodCode
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a point." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const QgsAbstractGeometry *geom = sipCpp->constGet();
      if ( QgsWkbTypes::flatType( geom->simplifiedTypeRef()->wkbType() ) != QgsWkbTypes::Point )
      {
        PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a point. Only Point types are permitted." ).arg( QgsWkbTypes::displayString( geom->wkbType() ) ).toUtf8().constData() );
        sipIsErr = 1;
      }
      else
      {
        sipRes = sipConvertFromNewType( new QgsPointXY( sipCpp->asPoint() ), sipType_QgsPointXY, Py_None );
      }
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a polyline.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved line type
     * (such as a CircularString), it will be automatically segmentized.
     *
     * \warning If the geometry is not a single-line (or single-curve) type, an empty list will be returned.
     */
    QgsPolylineXY asPolyline() const;
#else

    /**
     * Returns the contents of the geometry as a polyline.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved line type
     * (such as a CircularString), it will be automatically segmentized.
     *
     * This method works only with single-line (or single-curve).
     *
     * \throws TypeError if the geometry is not a single-line type
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asPolyline() const SIP_TYPEHINT( QgsPolylineXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a polyline." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::geometryType( type ) != QgsWkbTypes::LineGeometry || QgsWkbTypes::isMultiType( type ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a polyline. Only single line or curve types are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector< QgsPointXY >" );
      sipRes = sipConvertFromNewType( new QgsPolylineXY( sipCpp->asPolyline() ), qvector_type, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a polygon.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved polygon type
     * (such as a CurvePolygon), it will be automatically segmentized.
     *
     * \warning If the geometry is not a single-polygon (or single-curve polygon) type, an empty list will be returned.
     */
    QgsPolygonXY asPolygon() const;
#else

    /**
     * Returns the contents of the geometry as a polygon.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved polygon type
     * (such as a CurvePolygon), it will be automatically segmentized.
     *
     * This method works only with single-polygon (or single-curve polygon) geometry types.
     *
     * \throws TypeError if the geometry is not a single-polygon type
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asPolygon() const SIP_TYPEHINT( QgsPolygonXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a polygon." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::geometryType( type ) != QgsWkbTypes::PolygonGeometry || QgsWkbTypes::isMultiType( type ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a polygon. Only single polygon or curve polygon types are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector<QVector<QgsPointXY>>" );
      sipRes = sipConvertFromNewType( new QgsPolygonXY( sipCpp->asPolygon() ), qvector_type, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a multi-point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * \warning If the geometry is not a multi-point type, an empty list will be returned.
     */
    QgsMultiPointXY asMultiPoint() const;
#else

    /**
     * Returns the contents of the geometry as a multi-point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * This method works only with multi-point geometry types.
     *
     * \throws TypeError if the geometry is not a multi-point type
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asMultiPoint() const SIP_TYPEHINT( QgsMultiPointXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a multipoint." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::geometryType( type ) != QgsWkbTypes::PointGeometry || !QgsWkbTypes::isMultiType( type ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a multipoint. Only multipoint types are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector< QgsPointXY >" );
      sipRes = sipConvertFromNewType( new QgsPolylineXY( sipCpp->asMultiPoint() ), qvector_type, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a multi-linestring.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved line type
     * (such as a MultiCurve), it will be automatically segmentized.
     *
     * \warning If the geometry is not a multi-linestring (or multi-curve linestring) type, an empty list will be returned.
     */
    QgsMultiPolylineXY asMultiPolyline() const;
#else

    /**
     * Returns the contents of the geometry as a multi-linestring.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved line type
     * (such as a MultiCurve), it will be automatically segmentized.
     *
     * This method works only with multi-linestring (or multi-curve) geometry types.
     *
     * \throws TypeError if the geometry is not a multi-linestring type
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asMultiPolyline() const SIP_TYPEHINT( QgsMultiPolylineXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a multilinestring." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::geometryType( type ) != QgsWkbTypes::LineGeometry || !QgsWkbTypes::isMultiType( type ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a multilinestring. Only multi linestring or curves are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector<QVector<QgsPointXY>>" );
      sipRes = sipConvertFromNewType( new QgsMultiPolylineXY( sipCpp->asMultiPolyline() ), qvector_type, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a multi-polygon.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved polygon type
     * (such as a MultiSurface), it will be automatically segmentized.
     *
     * \warning If the geometry is not a multi-polygon (or multi-curve polygon) type, an empty list will be returned.
     */
    QgsMultiPolygonXY asMultiPolygon() const;
#else

    /**
     * Returns the contents of the geometry as a multi-polygon.
     *
     * Any z or m values present in the geometry will be discarded. If the geometry is a curved polygon type
     * (such as a MultiSurface), it will be automatically segmentized.
     *
     * This method works only with multi-polygon (or multi-curve polygon) geometry types.
     *
     * \throws TypeError if the geometry is not a multi-polygon type
     * \throws ValueError if the geometry is null
     */
    SIP_PYOBJECT asMultiPolygon() const SIP_TYPEHINT( QgsMultiPolygonXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a multipolygon." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::geometryType( type ) != QgsWkbTypes::PolygonGeometry || !QgsWkbTypes::isMultiType( type ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a multipolygon. Only multi polygon or curves are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      const sipTypeDef *qvector_type = sipFindType( "QVector<QVector<QVector<QgsPointXY>>>" );
      sipRes = sipConvertFromNewType( new QgsMultiPolygonXY( sipCpp->asMultiPolygon() ), qvector_type, Py_None );
    }
    % End
#endif

    /**
     * Returns contents of the geometry as a list of geometries
     * \since QGIS 1.1
     */
    QVector<QgsGeometry> asGeometryCollection() const;

    /**
     * Returns contents of the geometry as a QPointF if wkbType is WKBPoint,
     * otherwise returns a null QPointF.
     * \since QGIS 2.7
     */
    QPointF asQPointF() const SIP_HOLDGIL;

    /**
     * Returns contents of the geometry as a QPolygonF.
     *
     * If geometry is a linestring, then the result will be an open QPolygonF.
     * If the geometry is a polygon, then the result will be a closed QPolygonF
     * of the geometry's exterior ring.
     *
     * If the geometry is a multi-part geometry, then only the first part will
     * be considered when converting to a QPolygonF.
     *
     * \since QGIS 2.7
     */
    QPolygonF asQPolygonF() const SIP_HOLDGIL;

    /**
     * Deletes a ring in polygon or multipolygon.
     * Ring 0 is outer ring and can't be deleted.
     * \returns TRUE on success
     * \since QGIS 1.2
     */
    bool deleteRing( int ringNum, int partNum = 0 );

    /**
     * Deletes part identified by the part number
     * \returns TRUE on success
     * \since QGIS 1.2
     */
    bool deletePart( int partNum );

    /**
     * Converts single type geometry into multitype geometry
     * e.g. a polygon into a multipolygon geometry with one polygon
     * If it is already a multipart geometry, it will return TRUE and
     * not change the geometry.
     *
     * \returns TRUE in case of success and FALSE else
     */
    bool convertToMultiType();

    /**
     * Converts multi type geometry into single type geometry
     * e.g. a multipolygon into a polygon geometry. Only the first part of the
     * multi geometry will be retained.
     * If it is already a single part geometry, it will return TRUE and
     * not change the geometry.
     *
     * \returns TRUE in case of success and FALSE else
     */
    bool convertToSingleType();

    /**
     * Converts geometry collection to a the desired geometry type subclass (multi-point,
     * multi-linestring or multi-polygon). Child geometries of different type are filtered out.
     * Does nothing the geometry is not a geometry collection. May leave the geometry
     * empty if none of the child geometries match the desired type.
     *
     * \returns TRUE in case of success and FALSE else
     * \since QGIS 3.2
     */
    bool convertGeometryCollectionToSubclass( QgsWkbTypes::GeometryType geomType );

    /**
     * Modifies geometry to avoid intersections with the layers specified in project properties
     * \param avoidIntersectionsLayers list of layers to check for intersections
     * \param ignoreFeatures possibility to give a list of features where intersections should be ignored (not available in Python bindings)
     * \returns 0 in case of success,
     *          1 if geometry is not of polygon type,
     *          2 if avoid intersection would change the geometry type,
     *          3 at least one geometry intersected is invalid. The algorithm may not work and return the same geometry as the input. You must fix your intersecting geometries.
     * \since QGIS 1.5
     */
    int avoidIntersections( const QList<QgsVectorLayer *> &avoidIntersectionsLayers,
                            const QHash<QgsVectorLayer *, QSet<QgsFeatureId> > &ignoreFeatures SIP_PYARGREMOVE = ( QHash<QgsVectorLayer *, QSet<QgsFeatureId> >() ) );

    /**
     * Attempts to make an invalid geometry valid without losing vertices.
     *
     * Already-valid geometries are returned without further intervention.
     * In case of full or partial dimensional collapses, the output geometry may be a collection
     * of lower-to-equal dimension geometries or a geometry of lower dimension.
     * Single polygons may become multi-geometries in case of self-intersections.
     * It preserves Z values, but M values will be dropped.
     *
     * If an error was encountered during the process, more information can be retrieved
     * by calling lastError() on the returned geometry.
     *
     * \returns new valid QgsGeometry or null geometry on error
     *
     * \note For QGIS builds using GEOS library versions older than 3.8 this method calls
     * an internal fork of PostGIS' ST_MakeValid() function. For builds based on GEOS 3.8 or
     * later this method calls the GEOS MakeValid method directly.
     *
     * \since QGIS 3.0
     */
    QgsGeometry makeValid() const;

    /**
     * Forces geometries to respect the Right-Hand-Rule, in which the area that is bounded by a polygon
     * is to the right of the boundary. In particular, the exterior ring is oriented in a clockwise direction
     * and the interior rings in a counter-clockwise direction.
     *
     * \warning Due to the conflicting definitions of the right-hand-rule in general use, it is recommended
     * to use the explicit forcePolygonClockwise() or forcePolygonCounterClockwise() methods instead.
     *
     * \see forcePolygonClockwise()
     * \see forcePolygonCounterClockwise()
     * \since QGIS 3.6
     */
    QgsGeometry forceRHR() const;

    /**
     * Forces geometries to respect the exterior ring is clockwise, interior rings are counter-clockwise convention.
     *
     * This convention is used primarily by ESRI software.
     *
     * \see forcePolygonCounterClockwise()
     * \since QGIS 3.24
     */
    QgsGeometry forcePolygonClockwise() const;

    /**
     * Forces geometries to respect the exterior ring is counter-clockwise, interior rings are clockwise convention.
     *
     * This convention matches the OGC Simple Features specification.
     *
     * \see forcePolygonClockwise()
     * \since QGIS 3.24
     */
    QgsGeometry forcePolygonCounterClockwise() const;

    /**
     * \ingroup core
     * \brief A geometry error.
     */
    class CORE_EXPORT Error
    {
      public:
        Error()
          : mMessage( QStringLiteral( "none" ) )
        {}

        explicit Error( const QString &m )
          : mMessage( m )
        {}

        Error( const QString &m, const QgsPointXY &p )
          : mMessage( m )
          , mLocation( p )
          , mHasLocation( true ) {}

        /**
         * A human readable error message containing details about the error.
         */
        QString what() const;

        /**
         * The coordinates at which the error is located and should be visualized.
         */
        QgsPointXY where() const;

        /**
         * TRUE if the location available from \see where is valid.
         */
        bool hasWhere() const;

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsGeometry.Error: %1>" ).arg( sipCpp->what() );
        sipRes = PyUnicode_FromString( str.toUtf8().data() );
        % End
#endif

        // TODO c++20 - replace with = default
        bool operator==( const QgsGeometry::Error &other ) const
        {
          return other.mMessage == mMessage && other.mHasLocation == mHasLocation && other.mLocation == mLocation;
        }

      private:
        QString mMessage;
        QgsPointXY mLocation;
        bool mHasLocation = false;
    };

    /**
     * Validates geometry and produces a list of geometry errors.
     * The \a method argument dictates which validator to utilize.
     *
     * The \a flags parameter indicates optional flags which control the type of validity checking performed.
     *
     * \since QGIS 1.5
     */
    void validateGeometry( QVector<QgsGeometry::Error> &errors SIP_OUT, Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const;

    /**
     * Reorganizes the geometry into a normalized form (or "canonical" form).
     *
     * Polygon rings will be rearranged so that their starting vertex is the lower left and ring orientation follows the
     * right hand rule, collections are ordered by geometry type, and other normalization techniques are applied. The
     * resultant geometry will be geometrically equivalent to the original geometry.
     *
     * \since QGIS 3.20
     */
    void normalize();

    /**
     * Compute the unary union on a list of \a geometries. May be faster than an iterative union on a set of geometries.
     * The returned geometry will be fully noded, i.e. a node will be created at every common intersection of the
     * input geometries. An empty geometry will be returned in the case of errors.
     */
    static QgsGeometry unaryUnion( const QVector<QgsGeometry> &geometries );

    /**
     * Creates a GeometryCollection geometry containing possible polygons formed from the constituent
     * linework of a set of \a geometries. The input geometries must be fully noded (i.e. nodes exist
     * at every common intersection of the geometries). The easiest way to ensure this is to first
     * call unaryUnion() on the set of input geometries and then pass the result to polygonize().
     * An empty geometry will be returned in the case of errors.
     * \since QGIS 3.0
     */
    static QgsGeometry polygonize( const QVector<QgsGeometry> &geometries );

    /**
     * Converts the geometry to straight line segments, if it is a curved geometry type.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
     * \see requiresConversionToStraightSegments
     * \since QGIS 2.10
     */
    void convertToStraightSegment( double tolerance = M_PI / 180., QgsAbstractGeometry::SegmentationToleranceType toleranceType = QgsAbstractGeometry::MaximumAngle );

    /**
     * Returns TRUE if the geometry is a curved geometry type which requires conversion to
     * display as straight line segments.
     * \see convertToStraightSegment
     * \since QGIS 2.10
     */
    bool requiresConversionToStraightSegments() const;

    /**
     * Transforms the geometry from map units to pixels in place.
     * \param mtp map to pixel transform
     * \since QGIS 2.10
     */
    void mapToPixel( const QgsMapToPixel &mtp );

    /**
     * Draws the geometry onto a QPainter
     * \param p destination QPainter
     * \since QGIS 2.10
     */
    void draw( QPainter &p ) const;

    /**
     * Calculates the vertex ID from a vertex \a number.
     *
     * If a matching vertex was found, it will be stored in \a id.
     *
     * Returns TRUE if vertex was found.
     *
     * \see vertexNrFromVertexId()
     * \since QGIS 2.10
     */
    bool vertexIdFromVertexNr( int number, QgsVertexId &id SIP_OUT ) const;

    /**
     * Returns the vertex number corresponding to a vertex \a id.
     *
     * The vertex numbers start at 0, so a return value of 0 corresponds
     * to the first vertex.
     *
     * Returns -1 if a corresponding vertex could not be found.
     *
     * \see vertexIdFromVertexNr()
     * \since QGIS 2.10
     */
    int vertexNrFromVertexId( QgsVertexId id ) const;

    /**
     * Returns an error string referring to the last error encountered
     * either when this geometry was created or when an operation
     * was performed on the geometry.
     *
     * \since QGIS 3.0
     */
    QString lastError() const SIP_HOLDGIL;

    /**
     * Filters the vertices from the geometry in place, removing any which do not return TRUE for the \a filter function
     * check. Has no effect when called on a single point geometry.
     *
     * Depending on the \a filter used, this may result in an invalid geometry.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.2
     */
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) SIP_SKIP;

    /**
     * Transforms the vertices from the geometry in place, applying the \a transform function
     * to every vertex.
     *
     * Depending on the \a transform used, this may result in an invalid geometry.
     *
     * Transform functions are not permitted to alter the dimensionality of vertices. If
     * a transform which adds (or removes) z/m values is desired, first call the corresponding
     * addZValue() or addMValue() function to change the geometry's dimensionality and then
     * transform.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.4
     */
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) SIP_SKIP;

    /**
     * Construct geometry from a QPointF
     * \param point source QPointF
     * \since QGIS 2.7
     */
    static QgsGeometry fromQPointF( QPointF point ) SIP_HOLDGIL;

    /**
     * Construct geometry from a QPolygonF. If the polygon is closed than
     * the resultant geometry will be a polygon, if it is open than the
     * geometry will be a polyline.
     * \param polygon source QPolygonF
     * \since QGIS 2.7
     */
    static QgsGeometry fromQPolygonF( const QPolygonF &polygon );

    /**
     * Creates a QgsPolylineXY from a QPolygonF.
     * \param polygon source polygon
     * \returns QgsPolylineXY
     * \see createPolygonFromQPolygonF
     * \deprecated use QgsGeometry::fromQPolygonF() or QgsLineString::fromQPolygonF() instead.
     */
    Q_DECL_DEPRECATED static QgsPolylineXY createPolylineFromQPolygonF( const QPolygonF &polygon ) SIP_DEPRECATED;

    /**
     * Creates a QgsPolygonXYfrom a QPolygonF.
     * \param polygon source polygon
     * \returns QgsPolygon
     * \see createPolylineFromQPolygonF
     * \deprecated use QgsGeometry::fromQPolygonF() or QgsLineString::fromQPolygonF() instead.
     */
    Q_DECL_DEPRECATED static QgsPolygonXY createPolygonFromQPolygonF( const QPolygonF &polygon ) SIP_DEPRECATED;

#ifndef SIP_RUN

    /**
     * Compares two polylines for equality within a specified tolerance.
     * \param p1 first polyline
     * \param p2 second polyline
     * \param epsilon maximum difference for coordinates between the polylines
     * \returns TRUE if polylines have the same number of points and all
     * points are equal within the specified tolerance
     * \since QGIS 2.9
     */
    static bool compare( const QgsPolylineXY &p1, const QgsPolylineXY &p2,
                         double epsilon = 4 * std::numeric_limits<double>::epsilon() );

    /**
     * Compares two polygons for equality within a specified tolerance.
     * \param p1 first polygon
     * \param p2 second polygon
     * \param epsilon maximum difference for coordinates between the polygons
     * \returns TRUE if polygons have the same number of rings, and each ring has the same
     * number of points and all points are equal within the specified tolerance
     * \since QGIS 2.9
     */
    static bool compare( const QgsPolygonXY &p1, const QgsPolygonXY &p2,
                         double epsilon = 4 * std::numeric_limits<double>::epsilon() );

    /**
     * Compares two multipolygons for equality within a specified tolerance.
     * \param p1 first multipolygon
     * \param p2 second multipolygon
     * \param epsilon maximum difference for coordinates between the multipolygons
     * \returns TRUE if multipolygons have the same number of polygons, the polygons have the same number
     * of rings, and each ring has the same number of points and all points are equal within the specified
     * tolerance
     * \since QGIS 2.9
     */
    static bool compare( const QgsMultiPolygonXY &p1, const QgsMultiPolygonXY &p2,
                         double epsilon = 4 * std::numeric_limits<double>::epsilon() );
#else

    /**
     * Compares two geometry objects for equality within a specified tolerance.
     * The objects can be of type QgsPolylineXY, QgsPolygonXYor QgsMultiPolygon.
     * The 2 types should match.
     * \param p1 first geometry object
     * \param p2 second geometry object
     * \param epsilon maximum difference for coordinates between the objects
     * \returns TRUE if objects are
     *
     * - polylines and have the same number of points and all
     *   points are equal within the specified tolerance
     * - polygons and have the same number of points and all
     *   points are equal within the specified tolerance
     * - multipolygons and  have the same number of polygons, the polygons have the same number
     *   of rings, and each ring has the same number of points and all points are equal
     *   within the specified tolerance
     *
     * \since QGIS 2.9
     */
    static bool compare( PyObject *obj1, PyObject *obj2, double epsilon = 4 * std::numeric_limits<double>::epsilon() );
    % MethodCode
    {
      sipRes = false;
      int state0;
      int state1;
      int sipIsErr = 0;

      if ( PyList_Check( a0 ) && PyList_Check( a1 ) &&
           PyList_GET_SIZE( a0 ) && PyList_GET_SIZE( a1 ) )
      {
        PyObject *o0 = PyList_GetItem( a0, 0 );
        PyObject *o1 = PyList_GetItem( a1, 0 );
        if ( o0 && o1 )
        {
          // compare polyline - polyline
          if ( sipCanConvertToType( o0, sipType_QgsPointXY, SIP_NOT_NONE ) &&
               sipCanConvertToType( o1, sipType_QgsPointXY, SIP_NOT_NONE ) &&
               sipCanConvertToType( a0, sipType_QVector_0100QgsPointXY, SIP_NOT_NONE ) &&
               sipCanConvertToType( a1, sipType_QVector_0100QgsPointXY, SIP_NOT_NONE ) )
          {
            QgsPolylineXY *p0;
            QgsPolylineXY *p1;
            p0 = reinterpret_cast<QgsPolylineXY *>( sipConvertToType( a0, sipType_QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state0, &sipIsErr ) );
            p1 = reinterpret_cast<QgsPolylineXY *>( sipConvertToType( a1, sipType_QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state1, &sipIsErr ) );
            if ( sipIsErr )
            {
              sipReleaseType( p0, sipType_QVector_0100QgsPointXY, state0 );
              sipReleaseType( p1, sipType_QVector_0100QgsPointXY, state1 );
            }
            else
            {
              sipRes = QgsGeometry::compare( *p0, *p1, a2 );
            }
          }
          else if ( PyList_Check( o0 ) && PyList_Check( o1 ) &&
                    PyList_GET_SIZE( o0 ) && PyList_GET_SIZE( o1 ) )
          {
            PyObject *oo0 = PyList_GetItem( o0, 0 );
            PyObject *oo1 = PyList_GetItem( o1, 0 );
            if ( oo0 && oo1 )
            {
              // compare polygon - polygon
              if ( sipCanConvertToType( oo0, sipType_QgsPointXY, SIP_NOT_NONE ) &&
                   sipCanConvertToType( oo1, sipType_QgsPointXY, SIP_NOT_NONE ) &&
                   sipCanConvertToType( a0, sipType_QVector_0600QVector_0100QgsPointXY, SIP_NOT_NONE ) &&
                   sipCanConvertToType( a1, sipType_QVector_0600QVector_0100QgsPointXY, SIP_NOT_NONE ) )
              {
                QgsPolygonXY *p0;
                QgsPolygonXY *p1;
                p0 = reinterpret_cast<QgsPolygonXY *>( sipConvertToType( a0, sipType_QVector_0600QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state0, &sipIsErr ) );
                p1 = reinterpret_cast<QgsPolygonXY *>( sipConvertToType( a1, sipType_QVector_0600QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state1, &sipIsErr ) );
                if ( sipIsErr )
                {
                  sipReleaseType( p0, sipType_QVector_0600QVector_0100QgsPointXY, state0 );
                  sipReleaseType( p1, sipType_QVector_0600QVector_0100QgsPointXY, state1 );
                }
                else
                {
                  sipRes = QgsGeometry::compare( *p0, *p1, a2 );
                }
              }
              else if ( PyList_Check( oo0 ) && PyList_Check( oo1 ) &&
                        PyList_GET_SIZE( oo0 ) && PyList_GET_SIZE( oo1 ) )
              {
                PyObject *ooo0 = PyList_GetItem( oo0, 0 );
                PyObject *ooo1 = PyList_GetItem( oo1, 0 );
                if ( ooo0 && ooo1 )
                {
                  // compare multipolygon - multipolygon
                  if ( sipCanConvertToType( ooo0, sipType_QgsPointXY, SIP_NOT_NONE ) &&
                       sipCanConvertToType( ooo1, sipType_QgsPointXY, SIP_NOT_NONE ) &&
                       sipCanConvertToType( a0, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, SIP_NOT_NONE ) &&
                       sipCanConvertToType( a1, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, SIP_NOT_NONE ) )
                  {
                    QgsMultiPolygonXY *p0;
                    QgsMultiPolygonXY *p1;
                    p0 = reinterpret_cast<QgsMultiPolygonXY *>( sipConvertToType( a0, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state0, &sipIsErr ) );
                    p1 = reinterpret_cast<QgsMultiPolygonXY *>( sipConvertToType( a1, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, 0, SIP_NOT_NONE, &state1, &sipIsErr ) );
                    if ( sipIsErr )
                    {
                      sipReleaseType( p0, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, state0 );
                      sipReleaseType( p1, sipType_QVector_0600QVector_0600QVector_0100QgsPointXY, state1 );
                    }
                    else
                    {
                      sipRes = QgsGeometry::compare( *p0, *p1, a2 );
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    % End
#endif

    /**
     * Smooths a geometry by rounding off corners using the Chaikin algorithm. This operation
     * roughly doubles the number of vertices in a geometry.
     *
     * If input geometries contain Z or M values, these will also be smoothed and the output
     * geometry will retain the same dimensionality as the input geometry.
     *
     * \param iterations number of smoothing iterations to run. More iterations results
     * in a smoother geometry
     * \param offset fraction of line to create new vertices along, between 0 and 1.0,
     * e.g., the default value of 0.25 will create new vertices 25% and 75% along each line segment
     * of the geometry for each iteration. Smaller values result in "tighter" smoothing.
     * \param minimumDistance minimum segment length to apply smoothing to
     * \param maxAngle maximum angle at node (0-180) at which smoothing will be applied
     * \since QGIS 2.9
     */
    QgsGeometry smooth( unsigned int iterations = 1, double offset = 0.25,
                        double minimumDistance = -1.0, double maxAngle = 180.0 ) const;

    /**
     * Creates and returns a new geometry engine representing the specified \a geometry.
     *
     * A geometry engine is a low-level representation of a QgsAbstractGeometry object, optimised for use with external
     * geometry libraries such as GEOS.
     *
     * QgsGeometryEngine objects provide a mechanism for optimized evaluation of geometric algorithms, including spatial relationships
     * between geometries and operations such as buffers or clipping. QgsGeometryEngine is recommended for use in any
     * performance critical code instead of directly using the equivalent QgsGeometry methods such as QgsGeometry::intersects().
     *
     * Many methods available in the QgsGeometryEngine class can benefit from pre-preparing geometries. For instance, whenever
     * a large number of spatial relationships will be tested (such as calling intersects(), within(), etc) then the
     * geometry should first be prepared by calling prepareGeometry() before performing the tests.
     *
     * ### Example
     *
     * \code{.py}
     *   # polygon_geometry contains a complex polygon, with many vertices
     *   polygon_geometry = QgsGeometry.fromWkt('Polygon((...))')
     *
     *   # create a QgsGeometryEngine representation of the polygon
     *   polygon_geometry_engine = QgsGeometry.createGeometryEngine(polygon_geometry.constGet())
     *
     *   # since we'll be performing many intersects tests, we can speed up these tests considerably
     *   # by first "preparing" the geometry engine
     *   polygon_geometry_engine.prepareGeometry()
     *
     *   # now we are ready to quickly test intersection against many other objects
     *   for feature in my_layer.getFeatures():
     *       feature_geometry = feature.geometry()
     *       # test whether the feature's geometry intersects our original complex polygon
     *       if polygon_geometry_engine.intersects(feature_geometry.constGet()):
     *           print('feature intersects the polygon!')
     * \endcode
     *
     * QgsGeometryEngine operations are backed by the GEOS library (https://trac.osgeo.org/geos/).
     */
    static QgsGeometryEngine *createGeometryEngine( const QgsAbstractGeometry *geometry ) SIP_FACTORY;

    /**
     * Upgrades a point list from QgsPointXY to QgsPoint
     * \param input list of QgsPointXY objects to be upgraded
     * \param output destination for list of points converted to QgsPoint
     */
    static void convertPointList( const QVector<QgsPointXY> &input, QgsPointSequence &output );

    /**
     * Downgrades a point list from QgsPoint to QgsPointXY
     * \param input list of QgsPoint objects to be downgraded
     * \param output destination for list of points converted to QgsPointXY
     */
    static void convertPointList( const QgsPointSequence &input, QVector<QgsPointXY> &output );

    //! Allows direct construction of QVariants from geometry.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QgsGeometryPrivate *d; //implicitly shared data pointer

    //! Last error encountered
    mutable QString mLastError;

    /**
     * Detaches the private geometry container from this instance, and clones
     * the existing geometry ready for modification.
     */
    void detach();

    /**
     * Detaches the private geometry container from this instance, and resets it
     * to a new abstract geometry pointer.
     */
    void reset( std::unique_ptr< QgsAbstractGeometry > newGeometry );

    static void convertPolygon( const QgsPolygon &input, QgsPolygonXY &output );

    //! Try to convert the geometry to a point
    QgsGeometry convertToPoint( bool destMultipart ) const;
    //! Try to convert the geometry to a line
    QgsGeometry convertToLine( bool destMultipart ) const;
    //! Try to convert the geometry to a polygon
    QgsGeometry convertToPolygon( bool destMultipart ) const;

    /**
     * Smooths a polyline using the Chaikin algorithm
     * \param line line to smooth
     * \param iterations number of smoothing iterations to run. More iterations results
     * in a smoother geometry
     * \param offset fraction of line to create new vertices along, between 0 and 1.0,
     * e.g., the default value of 0.25 will create new vertices 25% and 75% along each line segment
     * of the geometry for each iteration. Smaller values result in "tighter" smoothing.
     * \param minimumDistance minimum segment length to apply smoothing to
     * \param maxAngle maximum angle at node (0-180) at which smoothing will be applied
    */
    std::unique_ptr< QgsLineString > smoothLine( const QgsLineString &line, unsigned int iterations = 1, double offset = 0.25,
        double minimumDistance = -1, double maxAngle = 180.0 ) const;

    /**
     * Smooths a polygon using the Chaikin algorithm
     * \param polygon polygon to smooth
     * \param iterations number of smoothing iterations to run. More iterations results
     * in a smoother geometry
     * \param offset fraction of segment to create new vertices along, between 0 and 1.0,
     * e.g., the default value of 0.25 will create new vertices 25% and 75% along each line segment
     * of the geometry for each iteration. Smaller values result in "tighter" smoothing.
     * \param minimumDistance minimum segment length to apply smoothing to
     * \param maxAngle maximum angle at node (0-180) at which smoothing will be applied
    */
    std::unique_ptr< QgsPolygon > smoothPolygon( const QgsPolygon &polygon, unsigned int iterations = 1, double offset = 0.25,
        double minimumDistance = -1, double maxAngle = 180.0 ) const;


    friend class QgsInternalGeometryEngine;

}; // class QgsGeometry

Q_DECLARE_METATYPE( QgsGeometry )

//! Writes the geometry to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsGeometry &geometry );
//! Reads a geometry from stream in into geometry. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsGeometry &geometry );

#endif
