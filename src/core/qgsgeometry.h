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

#include <QString>
#include <QVector>
#include <QDomDocument>

#include "qgis.h"

#include <geos_c.h>

#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"

#include <QSet>

class QgsVectorLayer;

/** polyline is represented as a vector of points */
typedef QVector<QgsPoint> QgsPolyline;

/** polygon: first item of the list is outer ring, inner rings (if any) start from second item */
typedef QVector<QgsPolyline> QgsPolygon;

/** a collection of QgsPoints that share a common collection of attributes */
typedef QVector<QgsPoint> QgsMultiPoint;

/** a collection of QgsPolylines that share a common collection of attributes */
typedef QVector<QgsPolyline> QgsMultiPolyline;

/** a collection of QgsPolygons that share a common collection of attributes */
typedef QVector<QgsPolygon> QgsMultiPolygon;

class QgsRectangle;

/** \ingroup core
 * A geometry is the spatial representation of a feature.
 * Represents a geometry with input and output in formats specified by
 * (at least) the Open Geospatial Consortium (WKB / Wkt), and containing
 * various functions for geoprocessing of the geometry.
 *
 * The geometry is represented internally by the OGC WKB format or
 * as GEOS geometry. Some functions use WKB for their work, others
 * use GEOS.
 *
 * TODO: migrate completely to GEOS and only support WKB/Wkt import/export.
 *
 * @author Brendan Morley
 */
class QgsConstWkbPtr;
class QgsWkbPtr;

class CORE_EXPORT QgsGeometry
{
  public:
    //! Constructor
    QgsGeometry();

    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( const QgsGeometry & );

    /** assignments will prompt a deep copy of the object
      @note not available in python bindings
      */
    QgsGeometry & operator=( QgsGeometry const & rhs );

    //! Destructor
    ~QgsGeometry();

    /** return GEOS context handle
     * @note added in 2.6
     * @note not available in Python
     */
    static GEOSContextHandle_t getGEOSHandler();

    /** static method that creates geometry from Wkt */
    static QgsGeometry* fromWkt( QString wkt );

    /** construct geometry from a point */
    static QgsGeometry* fromPoint( const QgsPoint& point );
    /** construct geometry from a multipoint */
    static QgsGeometry* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** construct geometry from a polyline */
    static QgsGeometry* fromPolyline( const QgsPolyline& polyline );
    /** construct geometry from a multipolyline*/
    static QgsGeometry* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** construct geometry from a polygon */
    static QgsGeometry* fromPolygon( const QgsPolygon& polygon );
    /** construct geometry from a multipolygon */
    static QgsGeometry* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** construct geometry from a rectangle */
    static QgsGeometry* fromRect( const QgsRectangle& rect );
    /**Construct geometry from a QPointF
     * @param point source QPointF
     * @note added in QGIS 2.7
    */
    static QgsGeometry* fromQPointF( const QPointF& point );

    /**Construct geometry from a QPolygonF. If the polygon is closed than
     * the resultant geometry will be a polygon, if it is open than the
     * geometry will be a polyline.
     * @param polygon source QPolygonF
     * @note added in QGIS 2.7
    */
    static QgsGeometry* fromQPolygonF( const QPolygonF& polygon );

    /**
      Set the geometry, feeding in a geometry in GEOS format.
      This class will take ownership of the buffer.
      @note not available in python bindings
     */
    void fromGeos( GEOSGeometry* geos );
    /**
      Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
      This class will take ownership of the buffer.
     */
    void fromWkb( unsigned char * wkb, size_t length );

    /**
       Returns the buffer containing this geometry in WKB format.
       You may wish to use in conjunction with wkbSize().
    */
    const unsigned char* asWkb() const;

    /**
     * Returns the size of the WKB in asWkb().
     */
    size_t wkbSize() const;

    /**Returns a geos geometry. QgsGeometry keeps ownership, don't delete the returned object!
        @note not available in python bindings
      */
    const GEOSGeometry* asGeos() const;

    /** Returns type of wkb (point / linestring / polygon etc.) */
    QGis::WkbType wkbType() const;

    /** Returns type of the vector */
    QGis::GeometryType type() const;

    /** Returns true if wkb of the geometry is of WKBMulti* type */
    bool isMultipart() const;

    /** compare geometries using GEOS */
    bool isGeosEqual( QgsGeometry & );

    /** check validity using GEOS */
    bool isGeosValid();

    /** check if geometry is empty using GEOS */
    bool isGeosEmpty();

    /** get area of geometry using GEOS */
    double area();

    /** get length of geometry using GEOS */
    double length();

    double distance( QgsGeometry& geom );

    /**
       Returns the vertex closest to the given point, the corresponding vertex index, squared distance snap point / target point
    and the indices of the vertices before/after. The vertices before/after are -1 if not present
    */
    QgsPoint closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist );

    /**
       Returns the indexes of the vertices before and after the given vertex index.

       This function takes into account the following factors:

       1. If the given vertex index is at the end of a linestring,
          the adjacent index will be -1 (for "no adjacent vertex")
       2. If the given vertex index is at the end of a linear ring
          (such as in a polygon), the adjacent index will take into
          account the first vertex is equal to the last vertex (and will
          skip equal vertex positions).
    */
    void adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex );

    /** Insert a new vertex before the given vertex index,
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

    /** Moves the vertex at the given position number
     *  and item (first number is index 0)
     *  to the given coordinates.
     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry
     */
    bool moveVertex( double x, double y, int atVertex );

    /** Deletes the vertex at the given position number and item
     *  (first number is index 0)
     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point),
     *  or if the number of remaining verticies in the linestring
     *  would be less than two.
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool deleteVertex( int atVertex );

    /**
     *  Returns coordinates of a vertex.
     *  @param atVertex index of the vertex
     *  @return Coordinates of the vertex or QgsPoint(0,0) on error
     */
    QgsPoint vertexAt( int atVertex );

    /**
     *  Returns the squared cartesian distance between the given point
     *  to the given vertex index (vertex at the given position number,
     *  ring and item (first number is index 0))
     */
    double sqrDistToVertexAt( QgsPoint& point, int atVertex );

    /**
     * Searches for the closest vertex in this geometry to the given point.
     * @param point Specifiest the point for search
     * @param atVertex Receives index of the closest vertex
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestVertexWithContext( const QgsPoint& point, int& atVertex );

    /**
     * Searches for the closest segment of geometry to the given point
     * @param point Specifies the point for search
     * @param minDistPoint Receives the nearest point on the segment
     * @param afterVertex Receives index of the vertex after the closest segment. The vertex
     * before the closest segment is always afterVertex - 1
     * @param leftOf Out: Returns if the point lies on the left of right side of the segment ( < 0 means left, > 0 means right )
     * @param epsilon epsilon for segment snapping
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex, double* leftOf = 0, double epsilon = DEFAULT_SEGMENT_EPSILON );

    /**Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    int addRing( const QList<QgsPoint>& ring );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature*/
    int addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType = QGis::UnknownGeometry );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature
     @note not available in python bindings
     */
    int addPart( GEOSGeometry *newPart );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature
     @note available in python bindings as addPartGeometry (added in 2.2)
     */
    int addPart( QgsGeometry *newPart );

    /**Translate this geometry by dx, dy
     @return 0 in case of success*/
    int translate( double dx, double dy );

    /**Transform this geometry as described by CoordinateTranasform ct
     @return 0 in case of success*/
    int transform( const QgsCoordinateTransform& ct );

    /**Transform this geometry as described by QTransform ct
     @note added in 2.8
     @return 0 in case of success*/
    int transform( const QTransform& ct );

    /**Rotate this geometry around the Z axis
     @note added in 2.8
     @param rotation clockwise rotation in degrees
     @param center rotation center
     @return 0 in case of success*/
    int rotate( double rotation, const QgsPoint& center );

    /**Splits this geometry according to a given line. Note that the geometry is only split once. If there are several intersections
     between geometry and splitLine, only the first one is considered.
    @param splitLine the line that splits the geometry
    @param[out] newGeometries list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry( const QList<QgsPoint>& splitLine,
                       QList<QgsGeometry*>&newGeometries,
                       bool topological,
                       QList<QgsPoint> &topologyTestPoints );

    /**Replaces a part of this geometry with another line
      @return 0 in case of success */
    int reshapeGeometry( const QList<QgsPoint>& reshapeWithLine );

    /**Changes this geometry such that it does not intersect the other geometry
       @param other geometry that should not be intersect
       @return 0 in case of success*/
    int makeDifference( QgsGeometry* other );

    /**Returns the bounding box of this feature*/
    QgsRectangle boundingBox();

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects( const QgsRectangle& r ) const;

    /** Test for intersection with a geometry (uses GEOS) */
    bool intersects( const QgsGeometry* geometry ) const;

    /** Test for containment of a point (uses GEOS) */
    bool contains( const QgsPoint* p ) const;

    /** Test for if geometry is contained in another (uses GEOS) */
    bool contains( const QgsGeometry* geometry ) const;

    /** Test for if geometry is disjoint of another (uses GEOS) */
    bool disjoint( const QgsGeometry* geometry ) const;

    /** Test for if geometry equals another (uses GEOS) */
    bool equals( const QgsGeometry* geometry ) const;

    /** Test for if geometry touch another (uses GEOS) */
    bool touches( const QgsGeometry* geometry ) const;

    /** Test for if geometry overlaps another (uses GEOS) */
    bool overlaps( const QgsGeometry* geometry ) const;

    /** Test for if geometry is within another (uses GEOS) */
    bool within( const QgsGeometry* geometry ) const;

    /** Test for if geometry crosses another (uses GEOS) */
    bool crosses( const QgsGeometry* geometry ) const;

    /** Returns a buffer region around this geometry having the given width and with a specified number
        of segments used to approximate curves */
    QgsGeometry* buffer( double distance, int segments );

    /** Returns a buffer region around the geometry, with additional style options.
     * @param distance    buffer distance
     * @param segments    For round joins, number of segments to approximate quarter-circle
     * @param endCapStyle Round (1) / Flat (2) / Square (3) end cap style
     * @param joinStyle   Round (1) / Mitre (2) / Bevel (3) join style
     * @param mitreLimit  Limit on the mitre ratio used for very sharp corners
     * @note added in 2.4
     * @note needs GEOS >= 3.3 - otherwise always returns 0
     */
    QgsGeometry* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit );

    /** Returns an offset line at a given distance and side from an input line.
     * See buffer() method for details on parameters.
     * @note added in 2.4
     * @note needs GEOS >= 3.3 - otherwise always returns 0
     */
    QgsGeometry* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit );

    /** Returns a simplified version of this geometry using a specified tolerance value */
    QgsGeometry* simplify( double tolerance );

    /** Returns the center of mass of a geometry
    * @note for line based geometries, the center point of the line is returned,
    * and for point based geometries, the point itself is returned */
    QgsGeometry* centroid();

    /** Returns a point within a geometry */
    QgsGeometry* pointOnSurface();

    /** Returns the smallest convex polygon that contains all the points in the geometry. */
    QgsGeometry* convexHull();

    /* Return interpolated point on line at distance */
    QgsGeometry* interpolate( double distance );

    /** Returns a geometry representing the points shared by this geometry and other. */
    QgsGeometry* intersection( QgsGeometry* geometry );

    /** Returns a geometry representing all the points in this geometry and other (a
     * union geometry operation).
     * @note this operation is not called union since its a reserved word in C++.*/
    QgsGeometry* combine( QgsGeometry* geometry );

    /** Returns a geometry representing the points making up this geometry that do not make up other. */
    QgsGeometry* difference( QgsGeometry* geometry );

    /** Returns a Geometry representing the points making up this Geometry that do not make up other. */
    QgsGeometry* symDifference( QgsGeometry* geometry );

    /** Exports the geometry to WKT
     *  @note precision parameter added in 2.4
     *  @return true in case of success and false else
     */
    QString exportToWkt( const int &precision = 17 ) const;

    /** Exports the geometry to GeoJSON
     *  @return a QString representing the geometry as GeoJSON
     *  @note precision parameter added in 2.4
     */
    QString exportToGeoJSON( const int &precision = 17 ) const;

    /** try to convert the geometry to the requested type
     * @param destType the geometry type to be converted to
     * @param destMultipart determines if the output geometry will be multipart or not
     * @return the converted geometry or NULL pointer if the conversion fails.
     * @note added in 2.2
     */
    QgsGeometry* convertToType( QGis::GeometryType destType, bool destMultipart = false );

    /* Accessor functions for getting geometry data */

    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint() const;

    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline() const;

    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon() const;

    /** return contents of the geometry as a multi point
        if wkbType is WKBMultiPoint, otherwise an empty list */
    QgsMultiPoint asMultiPoint() const;

    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline() const;

    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon() const;

    /** return contents of the geometry as a list of geometries */
    QList<QgsGeometry*> asGeometryCollection() const;

    /**Return contents of the geometry as a QPointF if wkbType is WKBPoint,
     * otherwise returns a null QPointF.
     * @note added in QGIS 2.7
    */
    QPointF asQPointF() const;

    /**Return contents of the geometry as a QPolygonF. If geometry is a linestring,
     * then the result will be an open QPolygonF. If the geometry is a polygon,
     * then the result will be a closed QPolygonF of the geometry's exterior ring.
     * @note added in QGIS 2.7
    */
    QPolygonF asQPolygonF() const;

    /** delete a ring in polygon or multipolygon.
      Ring 0 is outer ring and can't be deleted.
      @return true on success */
    bool deleteRing( int ringNum, int partNum = 0 );

    /** delete part identified by the part number
      @return true on success */
    bool deletePart( int partNum );

    /**Converts single type geometry into multitype geometry
     e.g. a polygon into a multipolygon geometry with one polygon
    @return true in case of success and false else*/
    bool convertToMultiType();

    /** Modifies geometry to avoid intersections with the layers specified in project properties
     *  @return 0 in case of success,
     *          1 if geometry is not of polygon type,
     *          2 if avoid intersection would change the geometry type,
     *          3 other error during intersection removal
     *  @param ignoreFeatures possibility to give a list of features where intersections should be ignored (not available in python bindings)
     */
    int avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) );

    class Error
    {
        QString message;
        QgsPoint location;
        bool hasLocation;
      public:
        Error() : message( "none" ), hasLocation( false ) {}
        Error( QString m ) : message( m ), hasLocation( false ) {}
        Error( QString m, QgsPoint p ) : message( m ), location( p ), hasLocation( true ) {}

        QString what() { return message; };
        QgsPoint where() { return location; }
        bool hasWhere() { return hasLocation; }
    };

    /** Validate geometry and produce a list of geometry errors */
    void validateGeometry( QList<Error> &errors );

    /** compute the unary union on a list of geometries. May be faster than an iterative union on a set of geometries.
        @param geometryList a list of QgsGeometry* as input
        @returns the new computed QgsGeometry, or null
    */
    static QgsGeometry *unaryUnion( const QList<QgsGeometry*>& geometryList );

  private:
    // Private variables

    // All of these are mutable since there may be on-the-fly
    // conversions between WKB, GEOS and Wkt;
    // However the intent is the const functions do not
    // semantically change the value that this object represents.

    /** pointer to geometry in binary WKB format
        This is the class' native implementation
     */
    mutable unsigned char * mGeometry;

    /** size of geometry */
    mutable size_t mGeometrySize;

    /** cached GEOS version of this geometry */
    mutable GEOSGeometry* mGeos;

    /** If the geometry has been set since the last conversion to WKB **/
    mutable bool mDirtyWkb;

    /** If the geometry has been set  since the last conversion to GEOS **/
    mutable bool mDirtyGeos;


    // Private functions

    /** Converts from the WKB geometry to the GEOS geometry.
        @return   true in case of success and false else
     */
    bool exportWkbToGeos() const;

    /** Converts from the GEOS geometry to the WKB geometry.
        @return   true in case of success and false else
     */
    bool exportGeosToWkb() const;

    /** Insert a new vertex before the given vertex index (first number is index 0)
     *  in the given GEOS Coordinate Sequence.
     *  If the requested vertex number is greater
     *  than the last actual vertex,
     *  it is assumed that the vertex is to be appended instead of inserted.
     *  @param x x coordinate
     *  @param y y coordinate
     *  @param beforeVertex insert before vertex with this index
     *  @param old_sequence   The sequence to update (The caller remains the owner).
     *  @param new_sequence   The updated sequence (The caller becomes the owner if the function returns true).
     *  Returns false if beforeVertex does not correspond to a valid vertex number
     *  on the Coordinate Sequence.
     */
    bool insertVertex( double x, double y,
                       int beforeVertex,
                       const GEOSCoordSequence*  old_sequence,
                       GEOSCoordSequence** new_sequence );

    /**Transform a single vertex by QTransform
    @param wkbPtr pointer to current position in wkb array. Is increased automatically by the function
    @param trans transform matrix
    @param hasZValue 25D type?*/
    void transformVertex( QgsWkbPtr &wkbPtr, const QTransform& trans, bool hasZValue );

    /**Transforms a single vertex by ct.
    @param wkbPtr pointer to current position in wkb. Is increased automatically by the function
    @param ct the QgsCoordinateTransform
    @param hasZValue 25D type?*/
    void transformVertex( QgsWkbPtr &wkbPtr, const QgsCoordinateTransform& ct, bool hasZValue );

    //helper functions for geometry splitting

    /**Splits line/multiline geometries
     @param splitLine the line that splits the feature
     @param newGeometries new geometries if splitting was successful
     @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitLinearGeometry( GEOSGeometry *splitLine, QList<QgsGeometry*>& newGeometries );
    /**Splits polygon/multipolygon geometries
       @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitPolygonGeometry( GEOSGeometry *splitLine, QList<QgsGeometry*>& newGeometries );
    /**Splits line/multiline geometries following a single point*/
    GEOSGeometry* linePointDifference( GEOSGeometry* GEOSsplitPoint );

    /**Finds out the points that need to be tested for topological correctnes if this geometry will be split
     @return 0 in case of success*/
    int topologicalTestPointsSplit( const GEOSGeometry* splitLine, QList<QgsPoint>& testPoints ) const;

    /**Creates a new line from an original line and a reshape line. The part of the input line from the first to the last intersection with the
        reshape line will be replaced. The calling function takes ownership of the result.
    @param origLine the original line
    @param reshapeLineGeos the reshape line
    @return the reshaped line or 0 in case of error*/
    static GEOSGeometry* reshapeLine( const GEOSGeometry* origLine, const GEOSGeometry* reshapeLineGeos );

    /**Creates a new polygon replacing the part from the first to the second intersection with the reshape line. As a polygon ring is always closed,
        the method keeps the longer part of the existing boundary
    @param polygon geometry to reshape
    @param reshapeLineGeos the reshape line
    @return the reshaped polygon or 0 in case of error*/
    static GEOSGeometry* reshapePolygon( const GEOSGeometry* polygon, const GEOSGeometry* reshapeLineGeos );

    /**Nodes together a split line and a (multi-) polygon geometry in a multilinestring
     @return the noded multiline geometry or 0 in case of error. The calling function takes ownership of the node geometry*/
    static GEOSGeometry* nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *poly );

    /**Tests if line1 is completely contained in line2. This method works with a very small buffer around line2 and therefore is less prone
        to numerical errors as the corresponding geos method*/
    static int lineContainedInLine( const GEOSGeometry* line1, const GEOSGeometry* line2 );

    /**Tests if a point is on a line. This method works with a very small buffer and is thus less prone to numerical problems as the direct geos functions.
      @param point the point to test
      @param line line to test
      @return 0 not contained, 1 if contained, <0 in case of error*/
    static int pointContainedInLine( const GEOSGeometry* point, const GEOSGeometry* line );

    /** Determines the maximum number of digits before the dot */
    static int geomDigits( const GEOSGeometry* geom );

    /**Returns number of single geometry in a geos geometry. Is save for geos 2 and 3*/
    int numberOfGeometries( GEOSGeometry* g ) const;

    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult );

    /** return point from wkb */
    QgsPoint asPoint( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const;

    /** return polyline from wkb */
    QgsPolyline asPolyline( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const;

    /** return polygon from wkb */
    QgsPolygon asPolygon( QgsConstWkbPtr &wkbPtr, bool hasZValue ) const;

    static bool geosRelOp( char( *op )( GEOSContextHandle_t handle, const GEOSGeometry*, const GEOSGeometry * ),
                           const QgsGeometry* a, const QgsGeometry* b );

    /**Returns < 0 if point(x/y) is left of the line x1,y1 -> x1,y2*/
    double leftOf( double x, double y, double& x1, double& y1, double& x2, double& y2 );

    static inline bool moveVertex( QgsWkbPtr &wkbPtr, const double &x, const double &y, int atVertex, bool hasZValue, int &pointIndex, bool isRing );
    static inline int deleteVertex( QgsConstWkbPtr &srcPtr, QgsWkbPtr &dstPtr, int atVertex, bool hasZValue, int &pointIndex, bool isRing, bool lastItem );
    static inline bool insertVertex( QgsConstWkbPtr &srcPtr, QgsWkbPtr &dstPtr, int beforeVertex, const double &x, const double &y, bool hasZValue, int &pointIndex, bool isRing );

    /** try to convert the geometry to a point */
    QgsGeometry* convertToPoint( bool destMultipart );
    /** try to convert the geometry to a line */
    QgsGeometry* convertToLine( bool destMultipart );
    /** try to convert the geometry to a polygon */
    QgsGeometry* convertToPolygon( bool destMultipart );

    static QgsPolyline createPolylineFromQPolygonF( const QPolygonF &polygon );
    static QgsPolygon createPolygonFromQPolygonF( const QPolygonF &polygon );
}; // class QgsGeometry

Q_DECLARE_METATYPE( QgsGeometry );

class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;

  public:
    QgsWkbPtr( unsigned char *p ) { mP = p; }

    inline const QgsWkbPtr &operator>>( double &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( unsigned int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( char &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QGis::WkbType &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }

    inline QgsWkbPtr &operator<<( const double &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const unsigned int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const char &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QGis::WkbType &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }

    inline void operator+=( int n ) { mP += n; }

    inline operator unsigned char *() const { return mP; }
};

class CORE_EXPORT QgsConstWkbPtr
{
    mutable unsigned char *mP;

  public:
    QgsConstWkbPtr( const unsigned char *p ) { mP = ( unsigned char * ) p; }

    inline const QgsConstWkbPtr &operator>>( double &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( char &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QGis::WkbType &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }

    inline void operator+=( int n ) { mP += n; }

    inline operator const unsigned char *() const { return mP; }
};

#endif
