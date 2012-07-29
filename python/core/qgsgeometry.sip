
/** polyline is just a list of points */
typedef QVector<QgsPoint> QgsPolyline;

/** polygon: first item of the list is outer ring, inner rings (if any) start from second item */
typedef QVector< QVector<QgsPoint> > QgsPolygon;
    
/** a collection of QgsPoints that share a common collection of attributes */
typedef QVector<QgsPoint> QgsMultiPoint;

/** a collection of QgsPolylines that share a common collection of attributes */
typedef QVector< QVector<QgsPoint> > QgsMultiPolyline;

/** a collection of QgsPolygons that share a common collection of attributes */
typedef QVector< QVector< QVector<QgsPoint> > > QgsMultiPolygon;

typedef unsigned int size_t;

class QgsGeometry
{
%TypeHeaderCode
#include <qgsgeometry.h>
%End

  public:

    //! Constructor
    QgsGeometry();
    
    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( const QgsGeometry & );
    

    //! Destructor
    ~QgsGeometry();


    /** static method that creates geometry from Wkt */
    static QgsGeometry* fromWkt(QString wkt) /Factory/;
    
    /** construct geometry from a point */
    static QgsGeometry* fromPoint(const QgsPoint& point) /Factory/;
    /** construct geometry from a multipoint */
    static QgsGeometry* fromMultiPoint(const QgsMultiPoint& multipoint) /Factory/;
    /** construct geometry from a polyline */
    static QgsGeometry* fromPolyline(const QgsPolyline& polyline) /Factory/;
    /** construct geometry from a multipolyline*/
    static QgsGeometry* fromMultiPolyline(const QgsMultiPolyline& multiline) /Factory/;
    /** construct geometry from a polygon */
    static QgsGeometry* fromPolygon(const QgsPolygon& polygon) /Factory/;
    /** construct geometry from a multipolygon */
    static QgsGeometry* fromMultiPolygon(const QgsMultiPolygon& multipoly) /Factory/;
    /** construct geometry from a rectangle */
    static QgsGeometry* fromRect(const QgsRectangle& rect) /Factory/;
        

    
    /** 
       Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
       This class will take ownership of the buffer.
    */
    void fromWkb(unsigned char * wkb /Array/, size_t length /ArraySize/);
%MethodCode
  // create copy of Python's string and pass it to fromWkb()
  unsigned char * copy = new unsigned char[a1];
  memcpy(copy, a0, a1);
  sipCpp->fromWkb(copy, a1);
%End
    
    /** 
       Returns the buffer containing this geometry in WKB format.
       You may wish to use in conjunction with wkbSize().
    */
    SIP_PYOBJECT asWkb();
%MethodCode
  sipRes = PyString_FromStringAndSize((const char *)sipCpp->asWkb(), sipCpp->wkbSize());
%End
    
    /** 
       Returns the size of the WKB in asWkb().
    */
    size_t wkbSize();
    
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

    /**
       Set the geometry, feeding in a geometry in GEOS format.
    */
    // TODO: unsupported class... would be possible to use PyGEOS?
    //void fromGeos(geos::Geometry* geos);

    /** get area using GEOS
      @note added in 1.5
     */
    double area();

    /** get length using GEOS
      @note added in 1.5
     */
    double length();

    double distance(QgsGeometry& geom);

    /**
       Returns the vertex closest to the given point 
       (and also vertex index, squared distance and indexes of the vertices before/after)
    */
    QgsPoint closestVertex(const QgsPoint& point, int& atVertex /Out/, int& beforeVertex /Out/, int& afterVertex /Out/, double& sqrDist /Out/);

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
    void adjacentVertices(int atVertex, int& beforeVertex /Out/, int& afterVertex /Out/);

    /** Insert a new vertex before the given vertex index,
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
	bool insertVertex(double x, double y, int beforeVertex);

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates.
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry 
     */
	bool moveVertex(double x, double y, int atVertex);
    
	/** Deletes the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point),
     *  or if the number of remaining verticies in the linestring
     *  would be less than two.
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
	bool deleteVertex(int atVertex);

    /**
     *  Returns coordinates of a vertex.
     *  @param atVertex index of the vertex
     *  @return Coordinates of the vertex or QgsPoint(0,0) on error
     */
    QgsPoint vertexAt(int atVertex);

    /**
        Returns the squared cartesian distance between the given point
        to the given vertex index*/
	double sqrDistToVertexAt(QgsPoint& point /In/, int atVertex);

	
	/**
     * Searches for the the closest vertex in this geometry to the given point.
     * @param point Specifiest the point for search
     * @param atVertex Receives index of the closest vertex
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestVertexWithContext(const QgsPoint& point, int& atVertex /Out/);

    /**
     * Searches for the closest segment of geometry to the given point
     * @param point Specifies the point for search
     * @param minDistPoint Receives the nearest point on the segment
     * @param beforeVertex Receives index of the vertex before the closest segment
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext(const QgsPoint& point, QgsPoint& minDistPoint /Out/, int& beforeVertex /Out/);

 /**Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed, \
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    int addRing(const QList<QgsPoint>& ring);

 /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring \
not disjoint with existing polygons of the feature*/
    int addPart(const QList<QgsPoint>& ring);
    int addIsland( const QList<QgsPoint> &ring ) /Deprecated/;

 /**Translate this geometry by dx, dy
     @return 0 in case of success*/
    int translate(double dx, double dy);

 /**Transform this geometry as described by CoordinateTranasform ct
     @return 0 in case of success*/
    int transform( const QgsCoordinateTransform& ct );

 /**Splits this geometry according to a given line. Note that the geometry is only split once. If there are several intersections
    between geometry and splitLine, only the first one is considered.
    @param splitLine the line that splits the geometry
    @param newGeometrys OUT: list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @topologyTestPoints OUT: points that need to be tested for topological completeness in the dataset
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry(const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries /Out/, bool topological, QList<QgsPoint>& topologyTestPoints /Out/);

/**Replaces a part of this geometry with another line
      @return 0 in case of success
      @note: this function was added in version 1.3*/
    int reshapeGeometry( const QList<QgsPoint>& reshapeWithLine );

 /**Changes this geometry such that it does not intersect the other geometry
       @param other geometry that should not be intersect
       @return 0 in case of success*/
    int makeDifference(QgsGeometry* other);	

    /**Returns the bounding box of this feature*/
    QgsRectangle boundingBox();

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects(const QgsRectangle& r);
    /** Test for intersection with a geoemetry (uses GEOS) */
    bool intersects(QgsGeometry* geometry);

    /** Test for containment of a point (uses GEOS) */
    bool contains(QgsPoint* p);
    
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
    QgsGeometry* buffer(double distance, int segments) /Factory/;

    /** Returns a simplified version of this geometry using a specified tolerance value */
    QgsGeometry* simplify(double tolerance) /Factory/;
    
    /** Returns the center of mass of a geometry
    * @note for line based geometries, the center point of the line is returned,
    * and for point based geometries, the point itself is returned */
    QgsGeometry* centroid() /Factory/;

    /** Returns the smallest convex polygon that contains all the points in the geometry. */
    QgsGeometry* convexHull() /Factory/;

    QgsGeometry* interoplate( double distance ) /Factory/;
    
    /** Returns a geometry representing the points shared by this geometry and other. */
    QgsGeometry* intersection(QgsGeometry* geometry) /Factory/;
    
    /** Returns a geometry representing all the points in this geometry and other (a 
     * union geometry operation). 
     * @note this operation is not called union since its a reserved word in C++.*/
    QgsGeometry* combine( QgsGeometry* geometry ) /Factory/;
    
    /** Returns a geometry representing the points making up this geometry that do not make up other. */
    QgsGeometry* difference(QgsGeometry* geometry) /Factory/;
    
    /** Returns a Geometry representing the points making up this Geometry that do not make up other. */
    QgsGeometry* symDifference(QgsGeometry* geometry) /Factory/;
    
    /**Creates a geos geometry from this features geometry. Note, that the returned object needs to be deleted*/
    // TODO: unsupported class... would be possible to use PyGEOS?
    //geos::Geometry* geosGeometry() const;

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
        if wkbType is WKBMultiPint, otherwise an empty list */
    QgsMultiPoint asMultiPoint();
    
    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline();
    
    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon();

    /** return contents of the geometry as a list of geometries
     @note added in version 1.1 */
    // TODO: destruction of created geometries??
    QList<QgsGeometry*> asGeometryCollection() /Factory/;

    /** delete a ring in polygon or multipolygon.
      Ring 0 is outer ring and can't be deleted.
      @return TRUE on success
      @note added in version 1.2 */
    bool deleteRing( int ringNum, int partNum = 0 );

    /** delete part identified by the part number
      @return TRUE on success
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
      public:
        Error( QString m );
        Error( QString m, QgsPoint p );
        QString what();
        QgsPoint where();
        bool hasWhere();
    };
    
    /** Validate geometry and produce a list of geometry errors
     * @note python binding added in 1.6
     **/
    void validateGeometry( QList<QgsGeometry::Error> &errors /Out/ );
}; // class QgsGeometry

