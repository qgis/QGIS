/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometry.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/










typedef QVector<QgsPointXY> QgsPolylineXY;

typedef QVector<QgsPoint> QgsPolyline;

typedef QVector<QVector<QgsPointXY>> QgsPolygonXY;

typedef QVector<QgsPointXY> QgsMultiPointXY;

typedef QVector<QVector<QgsPointXY>> QgsMultiPolylineXY;

typedef QVector<QVector<QVector<QgsPointXY>>> QgsMultiPolygonXY;





class QgsGeometry
{
%Docstring
A geometry is the spatial representation of a feature. Since QGIS 2.10, QgsGeometry acts as a generic container
for geometry objects. QgsGeometry is implicitly shared, so making copies of geometries is inexpensive. The geometry
container class can also be stored inside a QVariant object.

The actual geometry representation is stored as a QgsAbstractGeometry within the container, and
can be accessed via the geometry() method or set using the setGeometry() method.
%End

%TypeHeaderCode
#include "qgsgeometry.h"
%End
  public:

    enum OperationResult
    {
      Success,
      NothingHappened,
      InvalidBaseGeometry,
      InvalidInputGeometryType,
      SelectionIsEmpty,
      SelectionIsGreaterThanOne,
      GeometryEngineError,
      LayerNotEditable,
      AddPartSelectedGeometryNotFound,
      AddPartNotMultiGeometry,
      AddRingNotClosed,
      AddRingNotValid,
      AddRingCrossesExistingRings,
      AddRingNotInExistingFeature,
      SplitCannotSplitPoint,
    };

    QgsGeometry();
%Docstring
Constructor
%End

    QgsGeometry( const QgsGeometry & );
%Docstring
Copy constructor will prompt a deep copy of the object
%End


    explicit QgsGeometry( QgsAbstractGeometry *geom /Transfer/ );
%Docstring
Creates a geometry from an abstract geometry object. Ownership of
geom is transferred.

.. versionadded:: 2.10
%End


    ~QgsGeometry();

    const QgsAbstractGeometry *constGet() const;
%Docstring
Returns a non-modifiable (const) reference to the underlying abstract geometry primitive.

This is much faster then calling the non-const get() method.

.. note::

   In QGIS 2.x this method was named geometry().

.. versionadded:: 3.0

.. seealso:: :py:func:`primitive`

.. seealso:: :py:func:`set`
%End

    QgsAbstractGeometry *get();
%Docstring
Returns a modifiable (non-const) reference to the underlying abstract geometry primitive.

This method can be slow to call, as it may trigger a detachment of the geometry
and a deep copy. Where possible, use constGet() instead.

.. note::

   In QGIS 2.x this method was named geometry().

.. versionadded:: 3.0

.. seealso:: :py:func:`constGet`

.. seealso:: :py:func:`set`
%End

    void set( QgsAbstractGeometry *geometry /Transfer/ );
%Docstring
Sets the underlying geometry store. Ownership of geometry is transferred.

.. note::

   In QGIS 2.x this method was named setGeometry().

.. versionadded:: 3.0

.. seealso:: :py:func:`get`

.. seealso:: :py:func:`constGet`
%End

    bool isNull() const;
%Docstring
Returns true if the geometry is null (ie, contains no underlying geometry
accessible via geometry() ).

.. seealso:: :py:func:`geometry`

.. versionadded:: 2.10

.. seealso:: :py:func:`isEmpty`
%End

    static QgsGeometry fromWkt( const QString &wkt );
%Docstring
Creates a new geometry from a WKT string
%End
    static QgsGeometry fromPointXY( const QgsPointXY &point );
%Docstring
Creates a new geometry from a QgsPointXY object
%End
    static QgsGeometry fromMultiPointXY( const QgsMultiPointXY &multipoint );
%Docstring
Creates a new geometry from a QgsMultiPointXY object
%End

    static QgsGeometry fromPolylineXY( const QgsPolylineXY &polyline );
%Docstring
Creates a new LineString geometry from a list of QgsPointXY points.

Using fromPolyline() is preferred, as fromPolyline() is more efficient
and will respect any Z or M dimensions present in the input points.

.. note::

   In QGIS 2.x this method was available as fromPolyline().

.. versionadded:: 3.0

.. seealso:: :py:func:`fromPolyline`
%End

    static QgsGeometry fromPolyline( const QgsPolyline &polyline );
%Docstring
Creates a new LineString geometry from a list of QgsPoint points.

This method will respect any Z or M dimensions present in the input points.
E.g. if input points are PointZ type, the resultant linestring will be
a LineStringZ type.

.. versionadded:: 3.0
%End

    static QgsGeometry fromMultiPolylineXY( const QgsMultiPolylineXY &multiline );
%Docstring
Creates a new geometry from a QgsMultiPolylineXY object
%End
    static QgsGeometry fromPolygonXY( const QgsPolygonXY &polygon );
%Docstring
Creates a new geometry from a :py:class:`QgsPolygon`
%End
    static QgsGeometry fromMultiPolygonXY( const QgsMultiPolygonXY &multipoly );
%Docstring
Creates a new geometry from a :py:class:`QgsMultiPolygon`
%End
    static QgsGeometry fromRect( const QgsRectangle &rect );
%Docstring
Creates a new geometry from a :py:class:`QgsRectangle`
%End
    static QgsGeometry collectGeometry( const QVector<QgsGeometry> &geometries );
%Docstring
Creates a new multipart geometry from a list of QgsGeometry objects
%End



    void fromWkb( const QByteArray &wkb );
%Docstring
Set the geometry, feeding in the buffer containing OGC Well-Known Binary

.. versionadded:: 3.0
%End


    QgsWkbTypes::Type wkbType() const;
%Docstring
Returns type of the geometry as a WKB type (point / linestring / polygon etc.)

.. seealso:: :py:func:`type`
%End

    QgsWkbTypes::GeometryType type() const;
%Docstring
Returns type of the geometry as a QgsWkbTypes.GeometryType

.. seealso:: :py:func:`wkbType`
%End

    bool isEmpty() const;
%Docstring
Returns true if the geometry is empty (eg a linestring with no vertices,
or a collection with no geometries). A null geometry will always
return true for isEmpty().

.. seealso:: :py:func:`isNull`
%End

    bool isMultipart() const;
%Docstring
Returns true if WKB of the geometry is of WKBMulti* type
%End

    bool equals( const QgsGeometry &geometry ) const;
%Docstring
Test if this geometry is exactly equal to another ``geometry``.

This is a strict equality check, where the underlying geometries must
have exactly the same type, component vertices and vertex order.

Calling this method is dramatically faster than the topological
equality test performed by isGeosEqual().

.. note::

   Comparing two null geometries will return false.

.. versionadded:: 1.5

.. seealso:: :py:func:`isGeosEqual`
%End

    bool isGeosEqual( const QgsGeometry & ) const;
%Docstring
Compares the geometry with another geometry using GEOS.

This method performs a slow, topological check, where geometries
are considered equal if all of the their component edges overlap. E.g.
lines with the same vertex locations but opposite direction will be
considered equal by this method.

Consider using the much faster, stricter equality test performed
by equals() instead.

.. note::

   Comparing two null geometries will return false.

.. versionadded:: 1.5

.. seealso:: :py:func:`equals`
%End

    bool isGeosValid() const;
%Docstring
Checks validity of the geometry using GEOS

.. versionadded:: 1.5
%End

    bool isSimple() const;
%Docstring
Determines whether the geometry is simple (according to OGC definition),
i.e. it has no anomalous geometric points, such as self-intersection or self-tangency.
Uses GEOS library for the test.

.. note::

   This is useful mainly for linestrings and linear rings. Polygons are simple by definition,
for checking anomalies in polygon geometries one can use isGeosValid().

.. versionadded:: 3.0
%End

    double area() const;
%Docstring
Returns the area of the geometry using GEOS

.. versionadded:: 1.5
%End

    double length() const;
%Docstring
Returns the length of geometry using GEOS

.. versionadded:: 1.5
%End

    double distance( const QgsGeometry &geom ) const;
%Docstring
Returns the minimum distance between this geometry and another geometry, using GEOS.
Will return a negative value if a geometry is missing.

:param geom: geometry to find minimum distance to
%End


    QgsVertexIterator vertices() const;
%Docstring
Returns Java-style iterator for traversal of vertices of the geometry

.. versionadded:: 3.0
%End

    double hausdorffDistance( const QgsGeometry &geom ) const;
%Docstring
Returns the Hausdorff distance between this geometry and ``geom``. This is basically a measure of how similar or dissimilar 2 geometries are.

This algorithm is an approximation to the standard Hausdorff distance. This approximation is exact or close enough for a large
subset of useful cases. Examples of these are:

- computing distance between Linestrings that are roughly parallel to each other,
and roughly equal in length. This occurs in matching linear networks.
- Testing similarity of geometries.

If the default approximate provided by this method is insufficient, use hausdorffDistanceDensify() instead.

In case of error -1 will be returned.

.. versionadded:: 3.0

.. seealso:: :py:func:`hausdorffDistanceDensify`
%End

    double hausdorffDistanceDensify( const QgsGeometry &geom, double densifyFraction ) const;
%Docstring
Returns the Hausdorff distance between this geometry and ``geom``. This is basically a measure of how similar or dissimilar 2 geometries are.

This function accepts a ``densifyFraction`` argument. The function performs a segment
densification before computing the discrete Hausdorff distance. The ``densifyFraction`` parameter
sets the fraction by which to densify each segment. Each segment will be split into a
number of equal-length subsegments, whose fraction of the total length is
closest to the given fraction.

This method can be used when the default approximation provided by hausdorffDistance()
is not sufficient. Decreasing the ``densifyFraction`` parameter will make the
distance returned approach the true Hausdorff distance for the geometries.

In case of error -1 will be returned.

.. versionadded:: 3.0

.. seealso:: :py:func:`hausdorffDistance`
%End

    QgsPointXY closestVertex( const QgsPointXY &point, int &atVertex /Out/, int &beforeVertex /Out/, int &afterVertex /Out/, double &sqrDist /Out/ ) const;

    double distanceToVertex( int vertex ) const;
%Docstring
Returns the distance along this geometry from its first vertex to the specified vertex.

:param vertex: vertex index to calculate distance to

:return: distance to vertex (following geometry), or -1 for invalid vertex numbers

.. versionadded:: 2.16
%End

    double angleAtVertex( int vertex ) const;
%Docstring
Returns the bisector angle for this geometry at the specified vertex.

:param vertex: vertex index to calculate bisector angle at

:return: bisector angle, in radians clockwise from north

.. versionadded:: 3.0

.. seealso:: :py:func:`interpolateAngle`
%End

    void adjacentVertices( int atVertex, int &beforeVertex /Out/, int &afterVertex /Out/ ) const;
%Docstring
Returns the indexes of the vertices before and after the given vertex index.

This function takes into account the following factors:

1. If the given vertex index is at the end of a linestring,
the adjacent index will be -1 (for "no adjacent vertex")
2. If the given vertex index is at the end of a linear ring
(such as in a polygon), the adjacent index will take into
account the first vertex is equal to the last vertex (and will
skip equal vertex positions).
%End

    bool insertVertex( double x, double y, int beforeVertex );
%Docstring
Insert a new vertex before the given vertex index,
ring and item (first number is index 0)
If the requested vertex number (beforeVertex.back()) is greater
than the last actual vertex on the requested ring and item,
it is assumed that the vertex is to be appended instead of inserted.
Returns false if atVertex does not correspond to a valid vertex
on this geometry (including if this geometry is a Point).
It is up to the caller to distinguish between
these error conditions.  (Or maybe we add another method to this
object to help make the distinction?)
%End

    bool insertVertex( const QgsPoint &point, int beforeVertex );
%Docstring
Insert a new vertex before the given vertex index,
ring and item (first number is index 0)
If the requested vertex number (beforeVertex.back()) is greater
than the last actual vertex on the requested ring and item,
it is assumed that the vertex is to be appended instead of inserted.
Returns false if atVertex does not correspond to a valid vertex
on this geometry (including if this geometry is a Point).
It is up to the caller to distinguish between
these error conditions.  (Or maybe we add another method to this
object to help make the distinction?)
%End

    bool moveVertex( double x, double y, int atVertex );
%Docstring
Moves the vertex at the given position number
and item (first number is index 0)
to the given coordinates.
Returns false if atVertex does not correspond to a valid vertex
on this geometry
%End

    bool moveVertex( const QgsPoint &p, int atVertex );
%Docstring
Moves the vertex at the given position number
and item (first number is index 0)
to the given coordinates.
Returns false if atVertex does not correspond to a valid vertex
on this geometry
%End

    bool deleteVertex( int atVertex );
%Docstring
Deletes the vertex at the given position number and item
(first number is index 0)

:return: false if atVertex does not correspond to a valid vertex
on this geometry (including if this geometry is a Point),
or if the number of remaining vertices in the linestring
would be less than two.
It is up to the caller to distinguish between
these error conditions.  (Or maybe we add another method to this
object to help make the distinction?)
%End

    QgsPoint vertexAt( int atVertex ) const;
%Docstring
Returns coordinates of a vertex.

:param atVertex: index of the vertex

:return: Coordinates of the vertex or QgsPoint(0,0) on error
%End

    double sqrDistToVertexAt( QgsPointXY &point /In/, int atVertex ) const;
%Docstring
Returns the squared Cartesian distance between the given point
to the given vertex index (vertex at the given position number,
ring and item (first number is index 0))
%End

    QgsGeometry nearestPoint( const QgsGeometry &other ) const;
%Docstring
Returns the nearest point on this geometry to another geometry.

.. versionadded:: 2.14

.. seealso:: :py:func:`shortestLine`
%End

    QgsGeometry shortestLine( const QgsGeometry &other ) const;
%Docstring
Returns the shortest line joining this geometry to another geometry.

.. versionadded:: 2.14

.. seealso:: :py:func:`nearestPoint`
%End

    double closestVertexWithContext( const QgsPointXY &point, int &atVertex /Out/ ) const;
%Docstring
Searches for the closest vertex in this geometry to the given point.

:param point: Specifiest the point for search
:param atVertex: Receives index of the closest vertex

:return: The squared Cartesian distance is also returned in sqrDist, negative number on error
%End

    double closestSegmentWithContext( const QgsPointXY &point, QgsPointXY &minDistPoint /Out/, int &afterVertex /Out/, int *leftOf /Out/ = 0, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;
%Docstring
Searches for the closest segment of geometry to the given point

:param point: Specifies the point for search
:param minDistPoint: Receives the nearest point on the segment
:param afterVertex: Receives index of the vertex after the closest segment. The vertex
before the closest segment is always afterVertex - 1
:param leftOf: Out: Returns if the point lies on the left of left side of the geometry ( < 0 means left, > 0 means right, 0 indicates
that the test was unsuccessful, e.g. for a point exactly on the line)
:param epsilon: epsilon for segment snapping

:return: The squared Cartesian distance is also returned in sqrDist, negative number on error
%End

    OperationResult addRing( const QVector<QgsPointXY> &ring );
%Docstring
Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.

:param ring: The ring to be added

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult addRing( QgsCurve *ring /Transfer/ );
%Docstring
Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.

:param ring: The ring to be added

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult addPart( const QVector<QgsPointXY> &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) /PyName=addPointsXY/;
%Docstring
Adds a new part to a the geometry.

:param points: points describing part to add
:param geomType: default geometry type to create if no existing geometry

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult addPart( const QgsPointSequence &points, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry ) /PyName=addPoints/;
%Docstring
Adds a new part to a the geometry.

:param points: points describing part to add
:param geomType: default geometry type to create if no existing geometry

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult addPart( QgsAbstractGeometry *part /Transfer/, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::UnknownGeometry );
%Docstring
Adds a new part to this geometry.

:param part: part to add (ownership is transferred)
:param geomType: default geometry type to create if no existing geometry

:return: OperationResult a result code: success or reason of failure
%End


    OperationResult addPart( const QgsGeometry &newPart ) /PyName=addPartGeometry/;
%Docstring
Adds a new island polygon to a multipolygon feature

:return: OperationResult a result code: success or reason of failure

.. note::

   available in python bindings as addPartGeometry
%End

    QgsGeometry removeInteriorRings( double minimumAllowedArea = -1 ) const;
%Docstring
Removes the interior rings from a (multi)polygon geometry. If the minimumAllowedArea
parameter is specified then only rings smaller than this minimum
area will be removed.

.. versionadded:: 3.0
%End

    OperationResult translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );
%Docstring
Translates this geometry by dx, dy, dz and dm.

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult transform( const QgsCoordinateTransform &ct );
%Docstring
Transforms this geometry as described by CoordinateTransform ct

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );
%Docstring
Transforms the x and y components of the geometry using a QTransform object ``t``.

Optionally, the geometry's z values can be scaled via ``zScale`` and translated via ``zTranslate``.
Similarly, m-values can be scaled via ``mScale`` and translated via ``mTranslate``.

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult rotate( double rotation, const QgsPointXY &center );
%Docstring
Rotate this geometry around the Z axis

:param rotation: clockwise rotation in degrees
:param center: rotation center

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult splitGeometry( const QVector<QgsPointXY> &splitLine, QVector<QgsGeometry> &newGeometries /Out/, bool topological, QVector<QgsPointXY> &topologyTestPoints /Out/ );
%Docstring
Splits this geometry according to a given line.

:param splitLine: the line that splits the geometry
\param[out] newGeometries list of new geometries that have been created with the split
:param topological: true if topological editing is enabled
\param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset

:return: OperationResult a result code: success or reason of failure
%End

    OperationResult reshapeGeometry( const QgsLineString &reshapeLineString );
%Docstring
Replaces a part of this geometry with another line

:return: OperationResult a result code: success or reason of failure
%End


    QgsGeometry makeDifference( const QgsGeometry &other ) const;
%Docstring
Returns the geometry formed by modifying this geometry such that it does not
intersect the other geometry.

:param other: geometry that should not be intersect

:return: difference geometry, or empty geometry if difference could not be calculated

.. versionadded:: 3.0
%End

    QgsRectangle boundingBox() const;
%Docstring
Returns the bounding box of the geometry.

.. seealso:: :py:func:`orientedMinimumBoundingBox`
%End

    QgsGeometry orientedMinimumBoundingBox( double &area /Out/, double &angle /Out/, double &width /Out/, double &height /Out/ ) const;
%Docstring
Returns the oriented minimum bounding box for the geometry, which is the smallest (by area)
rotated rectangle which fully encompasses the geometry. The area, angle (clockwise in degrees from North),
width and height of the rotated bounding box will also be returned.

.. versionadded:: 3.0

.. seealso:: :py:func:`boundingBox`
%End


    QgsGeometry minimalEnclosingCircle( QgsPointXY &center /Out/, double &radius /Out/, unsigned int segments = 36 ) const;
%Docstring
Returns the minimal enclosing circle for the geometry.

:param center: Center of the minimal enclosing circle returneds
:param radius: Radius of the minimal enclosing circle returned
:param segments: Number of segments used to segment geometry. :py:func:`QgsEllipse.toPolygon`

.. versionadded:: 3.0
%End


    QgsGeometry orthogonalize( double tolerance = 1.0E-8, int maxIterations = 1000, double angleThreshold = 15.0 ) const;
%Docstring
Attempts to orthogonalize a line or polygon geometry by shifting vertices to make the geometries
angles either right angles or flat lines. This is an iterative algorithm which will loop until
either the vertices are within a specified tolerance of right angles or a set number of maximum
iterations is reached. The angle threshold parameter specifies how close to a right angle or
straight line an angle must be before it is attempted to be straightened.

.. versionadded:: 3.0
%End

    QgsGeometry snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const;
%Docstring
Returns a new geometry with all points or vertices snapped to the closest point of the grid.

If the gridified geometry could not be calculated (or was totally collapsed) an empty geometry will be returned.
Note that snapping to grid may generate an invalid geometry in some corner cases.
It can also be thought as rounding the edges and it may be useful for removing errors.

:param hSpacing: Horizontal spacing of the grid (x axis). 0 to disable.
:param vSpacing: Vertical spacing of the grid (y axis). 0 to disable.
:param dSpacing: Depth spacing of the grid (z axis). 0 (default) to disable.
:param mSpacing: Custom dimension spacing of the grid (m axis). 0 (default) to disable.

.. versionadded:: 3.0
%End

    bool removeDuplicateNodes( double epsilon = 4 * DBL_EPSILON, bool useZValues = false );
%Docstring
Removes duplicate nodes from the geometry, wherever removing the nodes does not result in a
degenerate geometry.

The ``epsilon`` parameter specifies the tolerance for coordinates when determining that
vertices are identical.

By default, z values are not considered when detecting duplicate nodes. E.g. two nodes
with the same x and y coordinate but different z values will still be considered
duplicate and one will be removed. If ``useZValues`` is true, then the z values are
also tested and nodes with the same x and y but different z will be maintained.

Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g.
a multipoint geometry with overlapping points will not be changed by this method.

The function will return true if nodes were removed, or false if no duplicate nodes
were found.

.. versionadded:: 3.0
%End

    bool intersects( const QgsRectangle &r ) const;
%Docstring
Tests for intersection with a rectangle (uses GEOS)
%End

    bool intersects( const QgsGeometry &geometry ) const;
%Docstring
Tests for intersection with a geometry (uses GEOS)
%End

    bool contains( const QgsPointXY *p ) const;
%Docstring
Tests for containment of a point (uses GEOS)
%End

    bool contains( const QgsGeometry &geometry ) const;
%Docstring
Tests for if geometry is contained in another (uses GEOS)

.. versionadded:: 1.5
%End

    bool disjoint( const QgsGeometry &geometry ) const;
%Docstring
Tests for if geometry is disjoint of another (uses GEOS)

.. versionadded:: 1.5
%End

    bool touches( const QgsGeometry &geometry ) const;
%Docstring
Test for if geometry touch another (uses GEOS)

.. versionadded:: 1.5
%End

    bool overlaps( const QgsGeometry &geometry ) const;
%Docstring
Test for if geometry overlaps another (uses GEOS)

.. versionadded:: 1.5
%End

    bool within( const QgsGeometry &geometry ) const;
%Docstring
Test for if geometry is within another (uses GEOS)

.. versionadded:: 1.5
%End

    bool crosses( const QgsGeometry &geometry ) const;
%Docstring
Test for if geometry crosses another (uses GEOS)

.. versionadded:: 1.5
%End

    enum BufferSide
    {
      SideLeft,
      SideRight,
    };

    enum EndCapStyle
    {
      CapRound,
      CapFlat,
      CapSquare,
    };

    enum JoinStyle
    {
      JoinStyleRound,
      JoinStyleMiter,
      JoinStyleBevel,
    };

    QgsGeometry buffer( double distance, int segments ) const;
%Docstring
Returns a buffer region around this geometry having the given width and with a specified number
of segments used to approximate curves
%End

    QgsGeometry buffer( double distance, int segments, EndCapStyle endCapStyle, JoinStyle joinStyle, double miterLimit ) const;
%Docstring
Returns a buffer region around the geometry, with additional style options.

:param distance:    buffer distance
:param segments:    for round joins, number of segments to approximate quarter-circle
:param endCapStyle: end cap style
:param joinStyle:   join style for corners in geometry
:param miterLimit:  limit on the miter ratio used for very sharp corners (JoinStyleMiter only)

.. versionadded:: 2.4
%End

    QgsGeometry offsetCurve( double distance, int segments, JoinStyle joinStyle, double miterLimit ) const;
%Docstring
Returns an offset line at a given distance and side from an input line.

:param distance:    buffer distance
:param segments:    for round joins, number of segments to approximate quarter-circle
:param joinStyle:   join style for corners in geometry
:param miterLimit:  limit on the miter ratio used for very sharp corners (JoinStyleMiter only)

.. versionadded:: 2.4
%End

    QgsGeometry singleSidedBuffer( double distance, int segments, BufferSide side,
                                   JoinStyle joinStyle = JoinStyleRound,
                                   double miterLimit = 2.0 ) const;
%Docstring
Returns a single sided buffer for a (multi)line geometry. The buffer is only
applied to one side of the line.

:param distance: buffer distance
:param segments: for round joins, number of segments to approximate quarter-circle
:param side: side of geometry to buffer
:param joinStyle: join style for corners
:param miterLimit: limit on the miter ratio used for very sharp corners

:return: buffered geometry, or an empty geometry if buffer could not be
calculated

.. versionadded:: 3.0
%End

    QgsGeometry extendLine( double startDistance, double endDistance ) const;
%Docstring
Extends a (multi)line geometry by extrapolating out the start or end of the line
by a specified distance. Lines are extended using the bearing of the first or last
segment in the line.

.. versionadded:: 3.0
%End

    QgsGeometry simplify( double tolerance ) const;
%Docstring
Returns a simplified version of this geometry using a specified tolerance value
%End

    QgsGeometry densifyByCount( int extraNodesPerSegment ) const;
%Docstring
Returns a copy of the geometry which has been densified by adding the specified
number of extra nodes within each segment of the geometry.
If the geometry has z or m values present then these will be linearly interpolated
at the added nodes.
Curved geometry types are automatically segmentized by this routine.

.. versionadded:: 3.0

.. seealso:: :py:func:`densifyByDistance`
%End

    QgsGeometry densifyByDistance( double distance ) const;
%Docstring
Densifies the geometry by adding regularly placed extra nodes inside each segment
so that the maximum distance between any two nodes does not exceed the
specified ``distance``.
E.g. specifying a distance 3 would cause the segment [0 0] -> [10 0]
to be converted to [0 0] -> [2.5 0] -> [5 0] -> [7.5 0] -> [10 0], since
3 extra nodes are required on the segment and spacing these at 2.5 increments
allows them to be evenly spaced over the segment.
If the geometry has z or m values present then these will be linearly interpolated
at the added nodes.
Curved geometry types are automatically segmentized by this routine.

.. versionadded:: 3.0

.. seealso:: :py:func:`densifyByCount`
%End

    QgsGeometry centroid() const;
%Docstring
Returns the center of mass of a geometry.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.

.. note::

   for line based geometries, the center point of the line is returned,
and for point based geometries, the point itself is returned

.. seealso:: :py:func:`pointOnSurface`

.. seealso:: :py:func:`poleOfInaccessibility`
%End

    QgsGeometry pointOnSurface() const;
%Docstring
Returns a point guaranteed to lie on the surface of a geometry. While the centroid()
of a geometry may be located outside of the geometry itself (e.g., for concave shapes),
the point on surface will always be inside the geometry.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.

.. seealso:: :py:func:`centroid`

.. seealso:: :py:func:`poleOfInaccessibility`
%End

    QgsGeometry poleOfInaccessibility( double precision, double *distanceToBoundary /Out/ = 0 ) const;
%Docstring
Calculates the approximate pole of inaccessibility for a surface, which is the
most distant internal point from the boundary of the surface. This function
uses the 'polylabel' algorithm (Vladimir Agafonkin, 2016), which is an iterative
approach guaranteed to find the true pole of inaccessibility within a specified
tolerance. More precise tolerances require more iterations and will take longer
to calculate.
Optionally, the distance to the polygon boundary from the pole can be stored.

.. seealso:: :py:func:`centroid`

.. seealso:: :py:func:`pointOnSurface`

.. versionadded:: 3.0
%End

    QgsGeometry convexHull() const;
%Docstring
Returns the smallest convex polygon that contains all the points in the geometry.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.
%End

    QgsGeometry voronoiDiagram( const QgsGeometry &extent = QgsGeometry(), double tolerance = 0.0, bool edgesOnly = false ) const;
%Docstring
Creates a Voronoi diagram for the nodes contained within the geometry.

Returns the Voronoi polygons for the nodes contained within the geometry.
If ``extent`` is specified then it will be used as a clipping envelope for the diagram.
If no extent is set then the clipping envelope will be automatically calculated.
In either case the diagram will be clipped to the larger of the provided envelope
OR the envelope surrounding all input nodes.
The ``tolerance`` parameter specifies an optional snapping tolerance which can
be used to improve the robustness of the diagram calculation.
If ``edgesOnly`` is true than line string boundary geometries will be returned
instead of polygons.
An empty geometry will be returned if the diagram could not be calculated.

.. versionadded:: 3.0
%End

    QgsGeometry delaunayTriangulation( double tolerance = 0.0, bool edgesOnly = false ) const;
%Docstring
Returns the Delaunay triangulation for the vertices of the geometry.
The ``tolerance`` parameter specifies an optional snapping tolerance which can
be used to improve the robustness of the triangulation.
If ``edgesOnly`` is true than line string boundary geometries will be returned
instead of polygons.
An empty geometry will be returned if the diagram could not be calculated.

.. versionadded:: 3.0
%End

    QgsGeometry subdivide( int maxNodes = 256 ) const;
%Docstring
Subdivides the geometry. The returned geometry will be a collection containing subdivided parts
from the original geometry, where no part has more then the specified maximum number of nodes (``maxNodes``).

This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially
indexed and faster to perform further operations such as intersects on. The returned geometry parts may
not be valid and may contain self-intersections.

The minimum allowed value for ``maxNodes`` is 8.

Curved geometries will be segmentized before subdivision.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.

.. versionadded:: 3.0
%End

    QgsGeometry interpolate( double distance ) const;
%Docstring
Returns interpolated point on line at distance.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.

.. versionadded:: 2.0

.. seealso:: :py:func:`lineLocatePoint`
%End

    double lineLocatePoint( const QgsGeometry &point ) const;
%Docstring
Returns a distance representing the location along this linestring of the closest point
on this linestring geometry to the specified point. Ie, the returned value indicates
how far along this linestring you need to traverse to get to the closest location
where this linestring comes to the specified point.

:param point: point to seek proximity to

:return: distance along line, or -1 on error

.. note::

   only valid for linestring geometries

.. seealso:: :py:func:`interpolate`

.. versionadded:: 3.0
%End

    double interpolateAngle( double distance ) const;
%Docstring
Returns the angle parallel to the linestring or polygon boundary at the specified distance
along the geometry. Angles are in radians, clockwise from north.
If the distance coincides precisely at a node then the average angle from the segment either side
of the node is returned.

:param distance: distance along geometry

.. versionadded:: 3.0

.. seealso:: :py:func:`angleAtVertex`
%End

    QgsGeometry intersection( const QgsGeometry &geometry ) const;
%Docstring
Returns a geometry representing the points shared by this geometry and other.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.
%End

    QgsGeometry clipped( const QgsRectangle &rectangle );
%Docstring
Clips the geometry using the specified ``rectangle``.

Performs a fast, non-robust intersection between the geometry and
a ``rectangle``. The returned geometry may be invalid.

.. versionadded:: 3.0
%End

    QgsGeometry combine( const QgsGeometry &geometry ) const;
%Docstring
Returns a geometry representing all the points in this geometry and other (a
union geometry operation).

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.

.. note::

   this operation is not called union since its a reserved word in C++.
%End

    QgsGeometry mergeLines() const;
%Docstring
Merges any connected lines in a LineString/MultiLineString geometry and
converts them to single line strings.

:return: a LineString or MultiLineString geometry, with any connected lines
joined. An empty geometry will be returned if the input geometry was not a
MultiLineString geometry.

.. versionadded:: 3.0
%End

    QgsGeometry difference( const QgsGeometry &geometry ) const;
%Docstring
Returns a geometry representing the points making up this geometry that do not make up other.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.
%End

    QgsGeometry symDifference( const QgsGeometry &geometry ) const;
%Docstring
Returns a geometry representing the points making up this geometry that do not make up other.

If the input is a NULL geometry, the output will also be a NULL geometry.

If an error was encountered while creating the result, more information can be retrieved
by calling `error()` on the returned geometry.
%End

    QgsGeometry extrude( double x, double y );
%Docstring
Returns an extruded version of this geometry.
%End

    QByteArray asWkb() const;
%Docstring
Export the geometry to WKB

.. versionadded:: 3.0
%End

    QString asWkt( int precision = 17 ) const;
%Docstring
Exports the geometry to WKT

:return: true in case of success and false else

.. note::

   precision parameter added in QGIS 2.4
%End

    QString asJson( int precision = 17 ) const;
%Docstring
Exports the geometry to a GeoJSON string.
%End

    QgsGeometry convertToType( QgsWkbTypes::GeometryType destType, bool destMultipart = false ) const /Factory/;
%Docstring
Try to convert the geometry to the requested type

:param destType: the geometry type to be converted to
:param destMultipart: determines if the output geometry will be multipart or not

:return: the converted geometry or None if the conversion fails.

.. versionadded:: 2.2
%End


    QgsPointXY asPoint() const;
%Docstring
Returns contents of the geometry as a point
if wkbType is WKBPoint, otherwise returns [0,0]
%End

    QgsPolylineXY asPolyline() const;
%Docstring
Returns contents of the geometry as a polyline
if wkbType is WKBLineString, otherwise an empty list
%End

    QgsPolygonXY asPolygon() const;
%Docstring
Returns contents of the geometry as a polygon
if wkbType is WKBPolygon, otherwise an empty list
%End

    QgsMultiPointXY asMultiPoint() const;
%Docstring
Returns contents of the geometry as a multi point
if wkbType is WKBMultiPoint, otherwise an empty list
%End

    QgsMultiPolylineXY asMultiPolyline() const;
%Docstring
Returns contents of the geometry as a multi linestring
if wkbType is WKBMultiLineString, otherwise an empty list
%End

    QgsMultiPolygonXY asMultiPolygon() const;
%Docstring
Returns contents of the geometry as a multi polygon
if wkbType is WKBMultiPolygon, otherwise an empty list
%End

    QVector<QgsGeometry> asGeometryCollection() const;
%Docstring
Return contents of the geometry as a list of geometries

.. versionadded:: 1.1
%End

    QPointF asQPointF() const;
%Docstring
Returns contents of the geometry as a QPointF if wkbType is WKBPoint,
otherwise returns a null QPointF.

.. versionadded:: 2.7
%End

    QPolygonF asQPolygonF() const;
%Docstring
Returns contents of the geometry as a QPolygonF. If geometry is a linestring,
then the result will be an open QPolygonF. If the geometry is a polygon,
then the result will be a closed QPolygonF of the geometry's exterior ring.

.. versionadded:: 2.7
%End

    bool deleteRing( int ringNum, int partNum = 0 );
%Docstring
Deletes a ring in polygon or multipolygon.
Ring 0 is outer ring and can't be deleted.

:return: true on success

.. versionadded:: 1.2
%End

    bool deletePart( int partNum );
%Docstring
Deletes part identified by the part number

:return: true on success

.. versionadded:: 1.2
%End

    bool convertToMultiType();
%Docstring
Converts single type geometry into multitype geometry
e.g. a polygon into a multipolygon geometry with one polygon
If it is already a multipart geometry, it will return true and
not change the geometry.

:return: true in case of success and false else
%End

    bool convertToSingleType();
%Docstring
Converts multi type geometry into single type geometry
e.g. a multipolygon into a polygon geometry. Only the first part of the
multi geometry will be retained.
If it is already a single part geometry, it will return true and
not change the geometry.

:return: true in case of success and false else
%End

    int avoidIntersections( const QList<QgsVectorLayer *> &avoidIntersectionsLayers );

%Docstring
Modifies geometry to avoid intersections with the layers specified in project properties

:return: 0 in case of success,
1 if geometry is not of polygon type,
2 if avoid intersection would change the geometry type,
3 other error during intersection removal

:param avoidIntersectionsLayers: list of layers to check for intersections
:param ignoreFeatures: possibility to give a list of features where intersections should be ignored (not available in Python bindings)

.. versionadded:: 1.5
%End

    QgsGeometry makeValid() const;
%Docstring
Attempts to make an invalid geometry valid without losing vertices.

Already-valid geometries are returned without further intervention.
In case of full or partial dimensional collapses, the output geometry may be a collection
of lower-to-equal dimension geometries or a geometry of lower dimension.
Single polygons may become multi-geometries in case of self-intersections.
It preserves Z values, but M values will be dropped.

If an error was encountered during the process, more information can be retrieved
by calling `error()` on the returned geometry.

:return: new valid QgsGeometry or null geometry on error

.. note::

   Ported from PostGIS ST_MakeValid() and it should return equivalent results.


.. versionadded:: 3.0
%End

    class Error
{

%TypeHeaderCode
#include "qgsgeometry.h"
%End
      public:
        Error();

        explicit Error( const QString &m );

        Error( const QString &m, const QgsPointXY &p );
        QString what();
        QgsPointXY where();
        bool hasWhere();
    };

    enum ValidationMethod
    {
      ValidatorQgisInternal,
      ValidatorGeos,
    };

    void validateGeometry( QVector<QgsGeometry::Error> &errors /Out/, ValidationMethod method = ValidatorQgisInternal ) const;
%Docstring
Validates geometry and produces a list of geometry errors.
The ``method`` argument dictates which validator to utilize.

.. versionadded:: 1.5

.. note::

   Available in Python bindings since QGIS 1.6
*
%End

    static QgsGeometry unaryUnion( const QVector<QgsGeometry> &geometries );
%Docstring
Compute the unary union on a list of ``geometries``. May be faster than an iterative union on a set of geometries.
The returned geometry will be fully noded, i.e. a node will be created at every common intersection of the
input geometries. An empty geometry will be returned in the case of errors.
%End

    static QgsGeometry polygonize( const QVector<QgsGeometry> &geometries );
%Docstring
Creates a GeometryCollection geometry containing possible polygons formed from the constituent
linework of a set of ``geometries``. The input geometries must be fully noded (i.e. nodes exist
at every common intersection of the geometries). The easiest way to ensure this is to first
call unaryUnion() on the set of input geometries and then pass the result to polygonize().
An empty geometry will be returned in the case of errors.

.. versionadded:: 3.0
%End

    void convertToStraightSegment();
%Docstring
Converts the geometry to straight line segments, if it is a curved geometry type.

.. versionadded:: 2.10

.. seealso:: :py:func:`requiresConversionToStraightSegments`
%End

    bool requiresConversionToStraightSegments() const;
%Docstring
Returns true if the geometry is a curved geometry type which requires conversion to
display as straight line segments.

.. versionadded:: 2.10

.. seealso:: :py:func:`convertToStraightSegment`
%End

    void mapToPixel( const QgsMapToPixel &mtp );
%Docstring
Transforms the geometry from map units to pixels in place.

:param mtp: map to pixel transform

.. versionadded:: 2.10
%End

    void draw( QPainter &p ) const;
%Docstring
Draws the geometry onto a QPainter

:param p: destination QPainter

.. versionadded:: 2.10
%End

    bool vertexIdFromVertexNr( int number, QgsVertexId &id /Out/ ) const;
%Docstring
Calculates the vertex ID from a vertex ``number``.

If a matching vertex was found, it will be stored in ``id``.

Returns true if vertex was found.

.. versionadded:: 2.10

.. seealso:: :py:func:`vertexNrFromVertexId`
%End

    int vertexNrFromVertexId( QgsVertexId id ) const;
%Docstring
Returns the vertex number corresponding to a vertex ``id``.

The vertex numbers start at 0, so a return value of 0 corresponds
to the first vertex.

Returns -1 if a corresponding vertex could not be found.

.. versionadded:: 2.10

.. seealso:: :py:func:`vertexIdFromVertexNr`
%End

    QString lastError() const;
%Docstring
Returns an error string referring to the last error encountered
either when this geometry was created or when an operation
was performed on the geometry.

.. versionadded:: 3.0
%End


    static QgsGeometry fromQPointF( QPointF point );
%Docstring
Construct geometry from a QPointF

:param point: source QPointF

.. versionadded:: 2.7
%End

    static QgsGeometry fromQPolygonF( const QPolygonF &polygon );
%Docstring
Construct geometry from a QPolygonF. If the polygon is closed than
the resultant geometry will be a polygon, if it is open than the
geometry will be a polyline.

:param polygon: source QPolygonF

.. versionadded:: 2.7
%End

    static QgsPolylineXY createPolylineFromQPolygonF( const QPolygonF &polygon ) /Factory/;
%Docstring
Creates a QgsPolylineXY from a QPolygonF.

:param polygon: source polygon

:return: :py:class:`QgsPolylineXY`

.. seealso:: :py:func:`createPolygonFromQPolygonF`
%End

    static QgsPolygonXY createPolygonFromQPolygonF( const QPolygonF &polygon ) /Factory/;
%Docstring
Creates a QgsPolygonXYfrom a QPolygonF.

:param polygon: source polygon

:return: :py:class:`QgsPolygon`

.. seealso:: :py:func:`createPolylineFromQPolygonF`
%End


    static bool compare( PyObject *obj1, PyObject *obj2, double epsilon = 4 * DBL_EPSILON );
%Docstring
Compares two geometry objects for equality within a specified tolerance.
The objects can be of type :py:class:`QgsPolylineXY`, QgsPolygonXYor :py:class:`QgsMultiPolygon`.
The 2 types should match.

:param p1: first geometry object
:param p2: second geometry object
:param epsilon: maximum difference for coordinates between the objects

:return: true if objects are
- polylines and have the same number of points and all
points are equal within the specified tolerance
- polygons and have the same number of points and all
points are equal within the specified tolerance
- multipolygons and  have the same number of polygons, the polygons have the same number
of rings, and each ring has the same number of points and all points are equal
within the specified
tolerance

.. versionadded:: 2.9
%End
%MethodCode
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
%End

    QgsGeometry smooth( const unsigned int iterations = 1, const double offset = 0.25,
                        double minimumDistance = -1.0, double maxAngle = 180.0 ) const;
%Docstring
Smooths a geometry by rounding off corners using the Chaikin algorithm. This operation
roughly doubles the number of vertices in a geometry.

:param iterations: number of smoothing iterations to run. More iterations results
in a smoother geometry
:param offset: fraction of line to create new vertices along, between 0 and 1.0,
e.g., the default value of 0.25 will create new vertices 25% and 75% along each line segment
of the geometry for each iteration. Smaller values result in "tighter" smoothing.
:param minimumDistance: minimum segment length to apply smoothing to
:param maxAngle: maximum angle at node (0-180) at which smoothing will be applied

.. versionadded:: 2.9
%End

    static QgsGeometryEngine *createGeometryEngine( const QgsAbstractGeometry *geometry ) /Factory/;
%Docstring
Creates and returns a new geometry engine
%End

    static void convertPointList( const QVector<QgsPointXY> &input, QgsPointSequence &output );
%Docstring
Upgrades a point list from QgsPointXY to :py:class:`QgsPoint`

:param input: list of QgsPointXY objects to be upgraded
:param output: destination for list of points converted to :py:class:`QgsPoint`
%End

    static void convertPointList( const QgsPointSequence &input, QVector<QgsPointXY> &output );
%Docstring
Downgrades a point list from QgsPoint to :py:class:`QgsPointXY`

:param input: list of QgsPoint objects to be downgraded
:param output: destination for list of points converted to :py:class:`QgsPointXY`
%End

    operator QVariant() const;

    operator bool() const;

}; // class QgsGeometry



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometry.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
