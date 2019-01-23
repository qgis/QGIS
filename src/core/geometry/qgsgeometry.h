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
#include <QSet>
#include <QString>
#include <QVector>

#include <climits>
#include <limits>
#include <memory>

#include "qgis_core.h"
#include "qgis.h"

#include "qgsabstractgeometry.h"
#include "qgspointxy.h"
#include "qgspoint.h"
#include "qgsfeatureid.h"


class QgsGeometryEngine;
class QgsVectorLayer;
class QgsMapToPixel;
class QPainter;
class QgsPolygon;
class QgsLineString;

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
typedef QVector<QgsPoint> QgsPolyline;

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
 * A geometry is the spatial representation of a feature. Since QGIS 2.10, QgsGeometry acts as a generic container
 * for geometry objects. QgsGeometry is implicitly shared, so making copies of geometries is inexpensive. The geometry
 * container class can also be stored inside a QVariant object.
 *
 * The actual geometry representation is stored as a QgsAbstractGeometry within the container, and
 * can be accessed via the get() method or set using the set() method.
 */

class CORE_EXPORT QgsGeometry
{
    Q_GADGET
    Q_PROPERTY( bool isNull READ isNull )
    Q_PROPERTY( QgsWkbTypes::GeometryType type READ type )

  public:

    /**
     * Success or failure of a geometry operation.
     * This gives details about cause of failure.
     */
    enum OperationResult
    {
      Success = 0, //!< Operation succeeded
      NothingHappened = 1000, //!< Nothing happened, without any error
      InvalidBaseGeometry, //!< The base geometry on which the operation is done is invalid or empty
      InvalidInputGeometryType, //!< The input geometry (ring, part, split line, etc.) has not the correct geometry type
      SelectionIsEmpty, //!< No features were selected
      SelectionIsGreaterThanOne, //!< More than one features were selected
      GeometryEngineError, //!< Geometry engine misses a method implemented or an error occurred in the geometry engine
      LayerNotEditable, //!< Cannot edit layer
      /* Add part issues */
      AddPartSelectedGeometryNotFound, //!< The selected geometry cannot be found
      AddPartNotMultiGeometry, //!< The source geometry is not multi
      /* Add ring issues*/
      AddRingNotClosed, //!< The input ring is not closed
      AddRingNotValid, //!< The input ring is not valid
      AddRingCrossesExistingRings, //!< The input ring crosses existing rings (it is not disjoint)
      AddRingNotInExistingFeature, //!< The input ring doesn't have any existing ring to fit into
      /* Split features */
      SplitCannotSplitPoint, //!< Cannot split points
    };

    //! Constructor
    QgsGeometry();

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

    ~QgsGeometry();

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
    const QgsAbstractGeometry *constGet() const;

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
     *
     * \see get()
     * \see constGet()
     * \since QGIS 3.0
     */
    void set( QgsAbstractGeometry *geometry SIP_TRANSFER );

    /**
     * Returns true if the geometry is null (ie, contains no underlying geometry
     * accessible via geometry() ).
     * \see get
     * \see isEmpty()
     * \since QGIS 2.10
     */
    bool isNull() const;

    //! Creates a new geometry from a WKT string
    static QgsGeometry fromWkt( const QString &wkt );
    //! Creates a new geometry from a QgsPointXY object
    static QgsGeometry fromPointXY( const QgsPointXY &point );
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

    //! Creates a new geometry from a QgsMultiPolylineXY object
    static QgsGeometry fromMultiPolylineXY( const QgsMultiPolylineXY &multiline );
    //! Creates a new geometry from a QgsPolygon
    static QgsGeometry fromPolygonXY( const QgsPolygonXY &polygon );
    //! Creates a new geometry from a QgsMultiPolygon
    static QgsGeometry fromMultiPolygonXY( const QgsMultiPolygonXY &multipoly );
    //! Creates a new geometry from a QgsRectangle
    static QgsGeometry fromRect( const QgsRectangle &rect );
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
    QgsWkbTypes::Type wkbType() const;

    /**
     * Returns type of the geometry as a QgsWkbTypes::GeometryType
     * \see wkbType
     */
    QgsWkbTypes::GeometryType type() const;

    /**
     * Returns true if the geometry is empty (eg a linestring with no vertices,
     * or a collection with no geometries). A null geometry will always
     * return true for isEmpty().
     * \see isNull()
     */
    bool isEmpty() const;

    //! Returns true if WKB of the geometry is of WKBMulti* type
    bool isMultipart() const;

    /**
     * Test if this geometry is exactly equal to another \a geometry.
     *
     * This is a strict equality check, where the underlying geometries must
     * have exactly the same type, component vertices and vertex order.
     *
     * Calling this method is dramatically faster than the topological
     * equality test performed by isGeosEqual().
     *
     * \note Comparing two null geometries will return false.
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
     * \note Comparing two null geometries will return false.
     *
     * \see equals()
     * \since QGIS 1.5
     */
    bool isGeosEqual( const QgsGeometry & ) const;

    /**
     * Checks validity of the geometry using GEOS
     * \since QGIS 1.5
     */
    bool isGeosValid() const;

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
     * Returns the area of the geometry using GEOS
     * \since QGIS 1.5
     */
    double area() const;

    /**
     * Returns the length of geometry using GEOS
     * \since QGIS 1.5
     */
    double length() const;

    /**
     * Returns the minimum distance between this geometry and another geometry, using GEOS.
     * Will return a negative value if a geometry is missing.
     *
     * \param geom geometry to find minimum distance to
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
     * * Example:
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
     * * Example:
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
     * * Example:
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
     *   # part iteration can also be combined with vertex iteration
     *   geometry = QgsGeometry.fromWkt( 'MultiPolygon((( 0 0, 0 10, 10 10, 10 0, 0 0 ),( 5 5, 5 6, 6 6, 6 5, 5 5)),((20 2, 22 2, 22 4, 20 4, 20 2)))' )
     *   for part in geometry.parts():
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
     * and roughly equal in length. This occurs in matching linear networks.
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
     * Returns the vertex closest to the given point, the corresponding vertex index, squared distance snap point / target point
     * and the indices of the vertices before and after the closest vertex.
     * \param point point to search for
     * \param atVertex will be set to the vertex index of the closest found vertex
     * \param beforeVertex will be set to the vertex index of the previous vertex from the closest one. Will be set to -1 if
     * not present.
     * \param afterVertex will be set to the vertex index of the next vertex after the closest one. Will be set to -1 if
     * not present.
     * \param sqrDist will be set to the square distance between the closest vertex and the specified point
     * \returns closest point in geometry. If not found (empty geometry), returns null point nad sqrDist is negative.
     */
    //TODO QGIS 3.0 - rename beforeVertex to previousVertex, afterVertex to nextVertex
    QgsPointXY closestVertex( const QgsPointXY &point, int &atVertex SIP_OUT, int &beforeVertex SIP_OUT, int &afterVertex SIP_OUT, double &sqrDist SIP_OUT ) const;

    /**
     * Returns the distance along this geometry from its first vertex to the specified vertex.
     * \param vertex vertex index to calculate distance to
     * \returns distance to vertex (following geometry), or -1 for invalid vertex numbers
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
     * 1. If the given vertex index is at the end of a linestring,
     *    the adjacent index will be -1 (for "no adjacent vertex")
     * 2. If the given vertex index is at the end of a linear ring
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
     *  Returns false if atVertex does not correspond to a valid vertex
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
     *  Returns false if atVertex does not correspond to a valid vertex
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
     * Returns false if atVertex does not correspond to a valid vertex
     * on this geometry
     */
    bool moveVertex( double x, double y, int atVertex );

    /**
     * Moves the vertex at the given position number
     * and item (first number is index 0)
     * to the given coordinates.
     * Returns false if atVertex does not correspond to a valid vertex
     * on this geometry
     */
    bool moveVertex( const QgsPoint &p, int atVertex );

    /**
     * Deletes the vertex at the given position number and item
     * (first number is index 0)
     * \returns false if atVertex does not correspond to a valid vertex
     * on this geometry (including if this geometry is a Point),
     * or if the number of remaining vertices in the linestring
     * would be less than two.
     * It is up to the caller to distinguish between
     * these error conditions.  (Or maybe we add another method to this
     * object to help make the distinction?)
     */
    bool deleteVertex( int atVertex );

    /**
     * Returns coordinates of a vertex.
     * \param atVertex index of the vertex
     * \returns Coordinates of the vertex or QgsPoint(0,0) on error
     */
    QgsPoint vertexAt( int atVertex ) const;

    /**
     * Returns the squared Cartesian distance between the given point
     * to the given vertex index (vertex at the given position number,
     * ring and item (first number is index 0))
     */
    double sqrDistToVertexAt( QgsPointXY &point SIP_IN, int atVertex ) const;

    /**
     * Returns the nearest point on this geometry to another geometry.
     * \see shortestLine()
     * \since QGIS 2.14
     */
    QgsGeometry nearestPoint( const QgsGeometry &other ) const;

    /**
     * Returns the shortest line joining this geometry to another geometry.
     * \see nearestPoint()
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
     * \param afterVertex Receives index of the vertex after the closest segment. The vertex
     * before the closest segment is always afterVertex - 1
     * \param leftOf Out: Returns if the point lies on the left of left side of the geometry ( < 0 means left, > 0 means right, 0 indicates
     * that the test was unsuccessful, e.g. for a point exactly on the line)
     * \param epsilon epsilon for segment snapping
     * \returns The squared Cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext( const QgsPointXY &point, QgsPointXY &minDistPoint SIP_OUT, int &afterVertex SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;

    /**
     * Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     * \param ring The ring to be added
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult addRing( const QVector<QgsPointXY> &ring );

    /**
     * Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     * \param ring The ring to be added
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult addRing( QgsCurve *ring SIP_TRANSFER );

    /**
     * Adds a new part to a the geometry.
     * \param points points describing part to add
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult addPart( const QVector<QgsPointXY> &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) SIP_PYNAME( addPointsXY );

    /**
     * Adds a new part to a the geometry.
     * \param points points describing part to add
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult addPart( const QgsPointSequence &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) SIP_PYNAME( addPoints );

    /**
     * Adds a new part to this geometry.
     * \param part part to add (ownership is transferred)
     * \param geomType default geometry type to create if no existing geometry
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult addPart( QgsAbstractGeometry *part SIP_TRANSFER, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );

    /**
     * Adds a new island polygon to a multipolygon feature
     * \returns OperationResult a result code: success or reason of failure
     * \note available in python bindings as addPartGeometry
     */
    OperationResult addPart( const QgsGeometry &newPart ) SIP_PYNAME( addPartGeometry );

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
    OperationResult translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );

    /**
     * Transforms this geometry as described by the coordinate transform \a ct.
     *
     * The transformation defaults to a forward transform, but the direction can be swapped
     * by setting the \a direction argument.
     *
     * By default, z-coordinates are not transformed, even if the coordinate transform
     * includes a vertical datum transformation. To transform z-coordinates, set
     * \a transformZ to true. This requires that the z coordinates in the geometry represent
     * height relative to the vertical datum of the source CRS (generally ellipsoidal heights)
     * and are expressed in its vertical units (generally meters).
     *
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection direction = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) SIP_THROW( QgsCsException );

    /**
     * Transforms the x and y components of the geometry using a QTransform object \a t.
     *
     * Optionally, the geometry's z values can be scaled via \a zScale and translated via \a zTranslate.
     * Similarly, m-values can be scaled via \a mScale and translated via \a mTranslate.
     *
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );

    /**
     * Rotate this geometry around the Z axis
     * \param rotation clockwise rotation in degrees
     * \param center rotation center
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult rotate( double rotation, const QgsPointXY &center );

    /**
     * Splits this geometry according to a given line.
     * \param splitLine the line that splits the geometry
     * \param[out] newGeometries list of new geometries that have been created with the split
     * \param topological true if topological editing is enabled
     * \param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult splitGeometry( const QVector<QgsPointXY> &splitLine, QVector<QgsGeometry> &newGeometries SIP_OUT, bool topological, QVector<QgsPointXY> &topologyTestPoints SIP_OUT );

    /**
     * Replaces a part of this geometry with another line
     * \returns OperationResult a result code: success or reason of failure
     */
    OperationResult reshapeGeometry( const QgsLineString &reshapeLineString );

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
     * \see boundingBox()
     * \since QGIS 3.0
     */
    QgsGeometry orientedMinimumBoundingBox( double &area SIP_OUT, double &angle SIP_OUT, double &width SIP_OUT, double &height SIP_OUT ) const;

    /**
     * Returns the oriented minimum bounding box for the geometry, which is the smallest (by area)
     * rotated rectangle which fully encompasses the geometry.
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
     * duplicate and one will be removed. If \a useZValues is true, then the z values are
     * also tested and nodes with the same x and y but different z will be maintained.
     *
     * Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g.
     * a multipoint geometry with overlapping points will not be changed by this method.
     *
     * The function will return true if nodes were removed, or false if no duplicate nodes
     * were found.
     *
     * \since QGIS 3.0
     */
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false );

    /**
     * Returns true if this geometry exactly intersects with a \a rectangle. This test is exact
     * and can be slow for complex geometries.
     *
     * The GEOS library is used to perform the intersection test. Geometries which are not
     * valid may return incorrect results.
     *
     * \see boundingBoxIntersects()
     */
    bool intersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns true if this geometry exactly intersects with another \a geometry. This test is exact
     * and can be slow for complex geometries.
     *
     * The GEOS library is used to perform the intersection test. Geometries which are not
     * valid may return incorrect results.
     *
     * \see boundingBoxIntersects()
     */
    bool intersects( const QgsGeometry &geometry ) const;

    /**
     * Returns true if the bounding box of this geometry intersects with a \a rectangle. Since this
     * test only considers the bounding box of the geometry, is is very fast to calculate and handles invalid
     * geometries.
     *
     * \see intersects()
     *
     * \since QGIS 3.0
     */
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const;

    /**
     * Returns true if the bounding box of this geometry intersects with the bounding box of another \a geometry. Since this
     * test only considers the bounding box of the geometries, is is very fast to calculate and handles invalid
     * geometries.
     *
     * \see intersects()
     *
     * \since QGIS 3.0
     */
    bool boundingBoxIntersects( const QgsGeometry &geometry ) const;

    //! Tests for containment of a point (uses GEOS)
    bool contains( const QgsPointXY *p ) const;

    /**
     * Tests for if geometry is contained in another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool contains( const QgsGeometry &geometry ) const;

    /**
     * Tests for if geometry is disjoint of another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool disjoint( const QgsGeometry &geometry ) const;

    /**
     * Test for if geometry touch another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool touches( const QgsGeometry &geometry ) const;

    /**
     * Test for if geometry overlaps another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool overlaps( const QgsGeometry &geometry ) const;

    /**
     * Test for if geometry is within another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool within( const QgsGeometry &geometry ) const;

    /**
     * Test for if geometry crosses another (uses GEOS)
     *  \since QGIS 1.5
     */
    bool crosses( const QgsGeometry &geometry ) const;

    //! Side of line to buffer
    enum BufferSide
    {
      SideLeft = 0, //!< Buffer to left of line
      SideRight, //!< Buffer to right of line
    };
    Q_ENUM( BufferSide )

    //! End cap styles for buffers
    enum EndCapStyle
    {
      CapRound = 1, //!< Round cap
      CapFlat, //!< Flat cap (in line with start/end of line)
      CapSquare, //!< Square cap (extends past start/end of line by buffer distance)
    };
    Q_ENUM( EndCapStyle )

    //! Join styles for buffers
    enum JoinStyle
    {
      JoinStyleRound = 1, //!< Use rounded joins
      JoinStyleMiter, //!< Use mitered joins
      JoinStyleBevel, //!< Use beveled joins
    };
    Q_ENUM( JoinStyle )

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
    QgsGeometry buffer( double distance, int segments, EndCapStyle endCapStyle, JoinStyle joinStyle, double miterLimit ) const;

    /**
     * Returns an offset line at a given distance and side from an input line.
     * \param distance    buffer distance
     * \param segments    for round joins, number of segments to approximate quarter-circle
     * \param joinStyle   join style for corners in geometry
     * \param miterLimit  limit on the miter ratio used for very sharp corners (JoinStyleMiter only)
     * \since QGIS 2.4
     */
    QgsGeometry offsetCurve( double distance, int segments, JoinStyle joinStyle, double miterLimit ) const;

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
    QgsGeometry singleSidedBuffer( double distance, int segments, BufferSide side,
                                   JoinStyle joinStyle = JoinStyleRound,
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
     * Returns the center of mass of a geometry.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling `error()` on the returned geometry.
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
     * by calling `error()` on the returned geometry.
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
     * Returns the smallest convex polygon that contains all the points in the geometry.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling `error()` on the returned geometry.
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
     * If \a edgesOnly is true than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry voronoiDiagram( const QgsGeometry &extent = QgsGeometry(), double tolerance = 0.0, bool edgesOnly = false ) const;

    /**
     * Returns the Delaunay triangulation for the vertices of the geometry.
     * The \a tolerance parameter specifies an optional snapping tolerance which can
     * be used to improve the robustness of the triangulation.
     * If \a edgesOnly is true than line string boundary geometries will be returned
     * instead of polygons.
     * An empty geometry will be returned if the diagram could not be calculated.
     * \since QGIS 3.0
     */
    QgsGeometry delaunayTriangulation( double tolerance = 0.0, bool edgesOnly = false ) const;

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
     * by calling `error()` on the returned geometry.
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
     * by calling `error()` on the returned geometry.
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
     * by calling `error()` on the returned geometry.
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
     * by calling `error()` on the returned geometry.
     */
    QgsGeometry difference( const QgsGeometry &geometry ) const;

    /**
     * Returns a geometry representing the points making up this geometry that do not make up other.
     *
     * If the input is a NULL geometry, the output will also be a NULL geometry.
     *
     * If an error was encountered while creating the result, more information can be retrieved
     * by calling `error()` on the returned geometry.
     */
    QgsGeometry symDifference( const QgsGeometry &geometry ) const;

    //! Returns an extruded version of this geometry.
    QgsGeometry extrude( double x, double y );

    /**
     * Export the geometry to WKB
     * \since QGIS 3.0
     */
    QByteArray asWkb() const;

    /**
     * Exports the geometry to WKT
     * \returns true in case of success and false else
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
     * Try to convert the geometry to the requested type
     * \param destType the geometry type to be converted to
     * \param destMultipart determines if the output geometry will be multipart or not
     * \returns the converted geometry or nullptr if the conversion fails.
     * \since QGIS 2.2
     */
    QgsGeometry convertToType( QgsWkbTypes::GeometryType destType, bool destMultipart = false ) const SIP_FACTORY;

    /* Accessor functions for getting geometry data */

#ifndef SIP_RUN

    /**
     * Returns the contents of the geometry as a 2-dimensional point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * \warning If the geometry is not a single-point type, a QgsPoint( 0, 0 ) will be returned.
     */
    QgsPointXY asPoint() const;
#else

    /**
     * Returns the contents of the geometry as a 2-dimensional point.
     *
     * Any z or m values present in the geometry will be discarded.
     *
     * This method works only with single-point geometry types. If the geometry
     * is not a single-point type, a TypeError will be raised. If the geometry
     * is null, a ValueError will be raised.
     */
    SIP_PYOBJECT asPoint() const SIP_TYPEHINT( QgsPointXY );
    % MethodCode
    const QgsWkbTypes::Type type = sipCpp->wkbType();
    if ( sipCpp->isNull() )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Null geometry cannot be converted to a point." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::Point )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "%1 geometry cannot be converted to a point. Only Point types are permitted." ).arg( QgsWkbTypes::displayString( type ) ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipConvertFromNewType( new QgsPointXY( sipCpp->asPoint() ), sipType_QgsPointXY, Py_None );
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
     * This method works only with single-line (or single-curve) geometry types. If the geometry
     * is not a single-line type, a TypeError will be raised. If the geometry is null, a ValueError
     * will be raised.
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
      const sipMappedType *qvector_type = sipFindMappedType( "QVector< QgsPointXY >" );
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
     * This method works only with single-polygon (or single-curve polygon) geometry types. If the geometry
     * is not a single-polygon type, a TypeError will be raised. If the geometry is null, a ValueError
     * will be raised.
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
      const sipMappedType *qvector_type = sipFindMappedType( "QVector<QVector<QgsPointXY>>" );
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
     * This method works only with multi-point geometry types. If the geometry
     * is not a multi-point type, a TypeError will be raised. If the geometry is null, a ValueError
     * will be raised.
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
      const sipMappedType *qvector_type = sipFindMappedType( "QVector< QgsPointXY >" );
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
     * This method works only with multi-linestring (or multi-curve) geometry types. If the geometry
     * is not a multi-linestring type, a TypeError will be raised. If the geometry is null, a ValueError
     * will be raised.
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
      const sipMappedType *qvector_type = sipFindMappedType( "QVector<QVector<QgsPointXY>>" );
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
     * This method works only with multi-polygon (or multi-curve polygon) geometry types. If the geometry
     * is not a multi-polygon type, a TypeError will be raised. If the geometry is null, a ValueError
     * will be raised.
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
      const sipMappedType *qvector_type = sipFindMappedType( "QVector<QVector<QVector<QgsPointXY>>>" );
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
    QPointF asQPointF() const;

    /**
     * Returns contents of the geometry as a QPolygonF. If geometry is a linestring,
     * then the result will be an open QPolygonF. If the geometry is a polygon,
     * then the result will be a closed QPolygonF of the geometry's exterior ring.
     * \since QGIS 2.7
     */
    QPolygonF asQPolygonF() const;

    /**
     * Deletes a ring in polygon or multipolygon.
     * Ring 0 is outer ring and can't be deleted.
     * \returns true on success
     * \since QGIS 1.2
     */
    bool deleteRing( int ringNum, int partNum = 0 );

    /**
     * Deletes part identified by the part number
     * \returns true on success
     * \since QGIS 1.2
     */
    bool deletePart( int partNum );

    /**
     * Converts single type geometry into multitype geometry
     * e.g. a polygon into a multipolygon geometry with one polygon
     * If it is already a multipart geometry, it will return true and
     * not change the geometry.
     *
     * \returns true in case of success and false else
     */
    bool convertToMultiType();

    /**
     * Converts multi type geometry into single type geometry
     * e.g. a multipolygon into a polygon geometry. Only the first part of the
     * multi geometry will be retained.
     * If it is already a single part geometry, it will return true and
     * not change the geometry.
     *
     * \returns true in case of success and false else
     */
    bool convertToSingleType();

    /**
     * Converts geometry collection to a the desired geometry type subclass (multi-point,
     * multi-linestring or multi-polygon). Child geometries of different type are filtered out.
     * Does nothing the geometry is not a geometry collection. May leave the geometry
     * empty if none of the child geometries match the desired type.
     *
     * \returns true in case of success and false else
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
     *          3 other error during intersection removal
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
     * by calling `error()` on the returned geometry.
     *
     * \returns new valid QgsGeometry or null geometry on error
     *
     * \note Ported from PostGIS ST_MakeValid() and it should return equivalent results.
     *
     * \since QGIS 3.0
     */
    QgsGeometry makeValid() const;

    /**
     * Forces geometries to respect the Right-Hand-Rule, in which the area that is bounded by a polygon
     * is to the right of the boundary. In particular, the exterior ring is oriented in a clockwise direction
     * and the interior rings in a counter-clockwise direction.
     *
     * \since QGIS 3.6
     */
    QgsGeometry forceRHR() const;

    /**
     * \ingroup core
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
         * True if the location available from \see where is valid.
         */
        bool hasWhere() const;

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsGeometry.Error: %1>" ).arg( sipCpp->what() );
        sipRes = PyUnicode_FromString( str.toUtf8().data() );
        % End
#endif

      private:
        QString mMessage;
        QgsPointXY mLocation;
        bool mHasLocation = false;
    };

    /**
     * Available methods for validating geometries.
     * \since QGIS 3.0
     */
    enum ValidationMethod
    {
      ValidatorQgisInternal, //!< Use internal QgsGeometryValidator method
      ValidatorGeos, //!< Use GEOS validation methods
    };

    /**
     * Validates geometry and produces a list of geometry errors.
     * The \a method argument dictates which validator to utilize.
     * \note Available in Python bindings since QGIS 1.6
     * \since QGIS 1.5
     */
    void validateGeometry( QVector<QgsGeometry::Error> &errors SIP_OUT, ValidationMethod method = ValidatorQgisInternal ) const;

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
     * Returns true if the geometry is a curved geometry type which requires conversion to
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
     * Returns true if vertex was found.
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
    QString lastError() const;

    /**
     * Filters the vertices from the geometry in place, removing any which do not return true for the \a filter function
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
    static QgsGeometry fromQPointF( QPointF point );

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
     */
    static QgsPolylineXY createPolylineFromQPolygonF( const QPolygonF &polygon ) SIP_FACTORY;

    /**
     * Creates a QgsPolygonXYfrom a QPolygonF.
     * \param polygon source polygon
     * \returns QgsPolygon
     * \see createPolylineFromQPolygonF
     */
    static QgsPolygonXY createPolygonFromQPolygonF( const QPolygonF &polygon ) SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Compares two polylines for equality within a specified tolerance.
     * \param p1 first polyline
     * \param p2 second polyline
     * \param epsilon maximum difference for coordinates between the polylines
     * \returns true if polylines have the same number of points and all
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
     * \returns true if polygons have the same number of rings, and each ring has the same
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
     * \returns true if multipolygons have the same number of polygons, the polygons have the same number
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
     * \returns true if objects are
     *   - polylines and have the same number of points and all
     *     points are equal within the specified tolerance
     *   - polygons and have the same number of points and all
     *     points are equal within the specified tolerance
     *   - multipolygons and  have the same number of polygons, the polygons have the same number
     *     of rings, and each ring has the same number of points and all points are equal
     *     within the specified
     * tolerance
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
     * Creates and returns a new geometry engine
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

    static void convertToPolyline( const QgsPointSequence &input, QgsPolylineXY &output );
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
