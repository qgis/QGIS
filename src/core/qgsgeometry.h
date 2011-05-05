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
/* $Id$ */

#ifndef QGSGEOMETRY_H
#define QGSGEOMETRY_H

#include <QString>
#include <QVector>

#include "qgis.h"

#include <geos_c.h>

#if defined(GEOS_VERSION_MAJOR) && (GEOS_VERSION_MAJOR<3)
#define GEOSGeometry struct GEOSGeom_t
#define GEOSCoordSequence struct GEOSCoordSeq_t
#endif

#include "qgspoint.h"
#include "qgscoordinatetransform.h"

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

class CORE_EXPORT QgsGeometry
{
  public:
    //! Constructor
    QgsGeometry();

    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( QgsGeometry const & );

    /** assignments will prompt a deep copy of the object */
    QgsGeometry & operator=( QgsGeometry const & rhs );

    //! Destructor
    ~QgsGeometry();

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
    /**
      Set the geometry, feeding in a geometry in GEOS format.
      This class will take ownership of the buffer.
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
    unsigned char * asWkb();

    /**
       Returns the size of the WKB in asWkb().
    */
    size_t wkbSize();

    /**Returns a geos geomtry. QgsGeometry keeps ownership, don't delete the returned object!
        @note this method was added in version 1.1*/
    GEOSGeometry* asGeos();

    /** Returns type of wkb (point / linestring / polygon etc.) */
    QGis::WkbType wkbType();

    /** Returns type of the vector */
    QGis::GeometryType type();

    /** Returns true if wkb of the geometry is of WKBMulti* type */
    bool isMultipart();

    /** compare geometries using GEOS
      @note added in 1.5
     */
    bool isGeosEqual( QgsGeometry & );

    /** check validity using GEOS
      @note added in 1.5
     */
    bool isGeosValid();

    /** check if geometry is empty using GEOS
      @note added in 1.5
     */
    bool isGeosEmpty();

    /** get area of geometry using GEOS
      @note added in 1.5
     */
    double area();

    /** get length of geometry using GEOS
      @note added in 1.5
     */
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
        Returns the squared cartesian distance between the given point
        to the given vertex index (vertex at the given position number,
        ring and item (first number is index 0))

     */
    double sqrDistToVertexAt( QgsPoint& point, int atVertex );

    /**
     * Searches for the the closest vertex in this geometry to the given point.
     * @param point Specifiest the point for search
     * @param atVertex Receives index of the closest vertex
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestVertexWithContext( const QgsPoint& point, int& atVertex );

    /**
     * Searches for the closest segment of geometry to the given point
     * @param point Specifies the point for search
     * @param minDistPoint Receives the nearest point on the segment
     * @param beforeVertex Receives index of the vertex before the closest segment. The vertex
     * after the closest segment is always beforeVertex + 1
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& beforeVertex );

    /**Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    int addRing( const QList<QgsPoint>& ring );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature*/
    int addPart( const QList<QgsPoint> &points );
    Q_DECL_DEPRECATED int addIsland( const QList<QgsPoint> &points );

    /**Translate this geometry by dx, dy
     @return 0 in case of success*/
    int translate( double dx, double dy );

    /**Transform this geometry as described by CoordinateTranasform ct
     @return 0 in case of success*/
    int transform( const QgsCoordinateTransform& ct );

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
      @return 0 in case of success
      @note: this function was added in version 1.3*/
    int reshapeGeometry( const QList<QgsPoint>& reshapeWithLine );

    /**Changes this geometry such that it does not intersect the other geometry
       @param other geometry that should not be intersect
       @return 0 in case of success*/
    int makeDifference( QgsGeometry* other );

    /**Returns the bounding box of this feature*/
    QgsRectangle boundingBox();

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects( const QgsRectangle& r );

    /** Test for intersection with a geometry (uses GEOS) */
    bool intersects( QgsGeometry* geometry );

    /** Test for containment of a point (uses GEOS) */
    bool contains( QgsPoint* p );

    /** Test for if geometry is contained in an other (uses GEOS)
     *  @note added in 1.5 */
    bool contains( QgsGeometry* geometry );

    /** Test for if geometry is disjoint of an other (uses GEOS)
     *  @note added in 1.5 */
    bool disjoint( QgsGeometry* geometry );

    /** Test for if geometry equals an other (uses GEOS)
     *  @note added in 1.5 */
    bool equals( QgsGeometry* geometry );

    /** Test for if geometry touch an other (uses GEOS)
     *  @note added in 1.5 */
    bool touches( QgsGeometry* geometry );

    /** Test for if geometry overlaps an other (uses GEOS)
     *  @note added in 1.5 */
    bool overlaps( QgsGeometry* geometry );

    /** Test for if geometry is within an other (uses GEOS)
     *  @note added in 1.5 */
    bool within( QgsGeometry* geometry );

    /** Test for if geometry crosses an other (uses GEOS)
     *  @note added in 1.5 */
    bool crosses( QgsGeometry* geometry );

    /** Returns a buffer region around this geometry having the given width and with a specified number
        of segments used to approximate curves */
    QgsGeometry* buffer( double distance, int segments );

    /** Returns a simplified version of this geometry using a specified tolerance value */
    QgsGeometry* simplify( double tolerance );

    /** Returns the center of mass of a geometry
    * @note for line based geometries, the center point of the line is returned,
    * and for point based geometries, the point itself is returned */
    QgsGeometry* centroid();

    /** Returns the smallest convex polygon that contains all the points in the geometry. */
    QgsGeometry* convexHull();

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

    /** Exports the geometry to mWkt
        @return true in case of success and false else
     */
    QString exportToWkt();

    /* Accessor functions for getting geometry data */

    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint();

    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline();

    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon();

    /** return contents of the geometry as a multi point
        if wkbType is WKBMultiPoint, otherwise an empty list */
    QgsMultiPoint asMultiPoint();

    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline();

    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon();

    /** return contents of the geometry as a list of geometries
     @note added in version 1.1 */
    QList<QgsGeometry*> asGeometryCollection();

    /** delete a ring in polygon or multipolygon.
      Ring 0 is outer ring and can't be deleted.
      @return true on success
      @note added in version 1.2 */
    bool deleteRing( int ringNum, int partNum = 0 );

    /** delete part identified by the part number
      @return true on success
      @note added in version 1.2 */
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
     *  @note added in 1.5
     */
    int avoidIntersections();


    class Error
    {
        QString message;
        QgsPoint location;
        bool hasLocation;
      public:
        Error( QString m ) : message( m ), hasLocation( false ) {}
        Error( QString m, QgsPoint p ) : message( m ), location( p ), hasLocation( true ) {}

        QString what() { return message; };
        QgsPoint where() { return location; }
        bool hasWhere() { return hasLocation; }
    };

    /** Validate geometry and produce a list of geometry errors
     * @note added in 1.5
     **/
    void validateGeometry( QList<Error> &errors );

    static void validatePolyline( QList<Error> &errors, int i, QgsPolyline polyline, bool ring = false );
    static void validatePolygon( QList<Error> &errors, int i, const QgsPolygon &polygon );

  private:
    // Private variables

    // All of these are mutable since there may be on-the-fly
    // conversions between WKB, GEOS and Wkt;
    // However the intent is the const functions do not
    // semantically change the value that this object represents.

    /** pointer to geometry in binary WKB format
        This is the class' native implementation
     */
    unsigned char * mGeometry;

    /** size of geometry */
    size_t mGeometrySize;

    /** cached GEOS version of this geometry */
    GEOSGeometry* mGeos;

    /** If the geometry has been set since the last conversion to WKB **/
    bool mDirtyWkb;

    /** If the geometry has been set  since the last conversion to GEOS **/
    bool mDirtyGeos;


    // Private functions

    /** Converts from the WKB geometry to the GEOS geometry.
        @return   true in case of success and false else
     */
    bool exportWkbToGeos();

    /** Converts from the GEOS geometry to the WKB geometry.
        @return   true in case of success and false else
     */
    bool exportGeosToWkb();

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

    /**Translates a single vertex by dx and dy.
    @param wkbPosition position in wkb array. Is increased automatically by the function
    @param dx translation of x-coordinate
    @param dy translation of y-coordinate
    @param hasZValue 25D type?*/
    void translateVertex( int& wkbPosition, double dx, double dy, bool hasZValue );

    /**Transforms a single vertex by ct.
    @param wkbPosition position in wkb array. Is increased automatically by the function
    @param ct the QgsCoordinateTransform
    @param hasZValue 25D type?*/
    void transformVertex( int& wkbPosition, const QgsCoordinateTransform& ct, bool hasZValue );

    //helper functions for geometry splitting

    /**Splits line/multiline geometries
     @param splitLine the line that splits the feature
     @param newGeometries new geometries if splitting was successful
     @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitLinearGeometry( GEOSGeometry *splitLine, QList<QgsGeometry*>& newGeometries );
    /**Splits polygon/multipolygon geometries
       @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitPolygonGeometry( GEOSGeometry *splitLine, QList<QgsGeometry*>& newGeometries );
    /**Finds out the points that need to be tested for topological correctnes if this geometry will be split
     @return 0 in case of success*/
    int topologicalTestPointsSplit( const GEOSGeometry* splitLine, QList<QgsPoint>& testPoints ) const;

    /**Creates a new line from an original line and a reshape line. The part of the input line from the first to the last intersection with the \
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

    /**Tests if geom bounding rect is within -180 <= x <= 180, -90 <= y <= 90. Other methods may use more accurate tolerances if this is true*/
    static bool geomInDegrees( const GEOSGeometry* geom );

    /**Returns number of single geometry in a geos geometry. Is save for geos 2 and 3*/
    int numberOfGeometries( GEOSGeometry* g ) const;

    int mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult );

    /** return point from wkb */
    QgsPoint asPoint( unsigned char*& ptr, bool hasZValue );

    /** return polyline from wkb */
    QgsPolyline asPolyline( unsigned char*& ptr, bool hasZValue );

    /** return polygon from wkb */
    QgsPolygon asPolygon( unsigned char*& ptr, bool hasZValue );

    static void checkRingIntersections( QList<Error> &errors,
                                        int p0, int i0, const QgsPolyline &ring0,
                                        int p1, int i1, const QgsPolyline &ring1 );

    static bool geosRelOp( char( *op )( const GEOSGeometry*, const GEOSGeometry * ),
                           QgsGeometry *a, QgsGeometry *b );


    static int refcount;
}; // class QgsGeometry

#endif
