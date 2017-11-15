/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsabstractgeometry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QVector< QgsPoint > QgsPointSequence;
typedef QVector< QVector< QgsPoint > > QgsRingSequence;
typedef QVector< QVector< QVector< QgsPoint > > > QgsCoordinateSequence;

class QgsAbstractGeometry
{
%Docstring
 Abstract base class for all geometries
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsabstractgeometry.h"
%End
%ConvertToSubClassCode
    if ( qgsgeometry_cast<QgsPoint *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPoint;
    else if ( qgsgeometry_cast<QgsLineString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsLineString;
    else if ( qgsgeometry_cast<QgsCircularString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCircularString;
    else if ( qgsgeometry_cast<QgsCompoundCurve *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCompoundCurve;
    else if ( qgsgeometry_cast<QgsTriangle *>( sipCpp ) != nullptr )
      sipType = sipType_QgsTriangle;
    else if ( qgsgeometry_cast<QgsPolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPolygon;
    else if ( qgsgeometry_cast<QgsCurvePolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsCurvePolygon;
    else if ( qgsgeometry_cast<QgsMultiPoint *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiPoint;
    else if ( qgsgeometry_cast<QgsMultiLineString *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiLineString;
    else if ( qgsgeometry_cast<QgsMultiPolygon *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiPolygon;
    else if ( qgsgeometry_cast<QgsMultiSurface *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiSurface;
    else if ( qgsgeometry_cast<QgsMultiCurve *>( sipCpp ) != nullptr )
      sipType = sipType_QgsMultiCurve;
    else if ( qgsgeometry_cast<QgsGeometryCollection *>( sipCpp ) != nullptr )
      sipType = sipType_QgsGeometryCollection;
    else
      sipType = 0;
%End
  public:

    enum SegmentationToleranceType
    {

      MaximumAngle,

      MaximumDifference
    };

    QgsAbstractGeometry();
%Docstring
 Constructor for QgsAbstractGeometry.
%End
    virtual ~QgsAbstractGeometry();
    QgsAbstractGeometry( const QgsAbstractGeometry &geom );

    virtual QgsAbstractGeometry *clone() const = 0 /Factory/;
%Docstring
 Clones the geometry by performing a deep copy
 :rtype: QgsAbstractGeometry
%End

    virtual void clear() = 0;
%Docstring
 Clears the geometry, ie reset it to a null geometry
%End

    virtual QgsRectangle boundingBox() const = 0;
%Docstring
 Returns the minimal bounding box for the geometry
 :rtype: QgsRectangle
%End


    virtual int dimension() const = 0;
%Docstring
 Returns the inherent dimension of the geometry. For example, this is 0 for a point geometry,
 1 for a linestring and 2 for a polygon.
 :rtype: int
%End

    virtual QString geometryType() const = 0;
%Docstring
 Returns a unique string representing the geometry type.
.. seealso:: wkbType
.. seealso:: wktTypeStr
 :rtype: str
%End

    QgsWkbTypes::Type wkbType() const;
%Docstring
 Returns the WKB type of the geometry.
.. seealso:: geometryType
.. seealso:: wktTypeStr
 :rtype: QgsWkbTypes.Type
%End

    QString wktTypeStr() const;
%Docstring
 Returns the WKT type string of the geometry.
.. seealso:: geometryType
.. seealso:: wkbType
 :rtype: str
%End

    bool is3D() const;
%Docstring
 Returns true if the geometry is 3D and contains a z-value.
.. seealso:: isMeasure
 :rtype: bool
%End

    bool isMeasure() const;
%Docstring
 Returns true if the geometry contains m values.
.. seealso:: is3D
 :rtype: bool
%End

    virtual QgsAbstractGeometry *boundary() const = 0 /Factory/;
%Docstring
 Returns the closure of the combinatorial boundary of the geometry (ie the topological boundary of the geometry).
 For instance, a polygon geometry will have a boundary consisting of the linestrings for each ring in the polygon.
 :return: boundary for geometry. May be null for some geometry types.
.. versionadded:: 3.0
 :rtype: QgsAbstractGeometry
%End


    virtual bool fromWkb( QgsConstWkbPtr &wkb ) = 0;
%Docstring
 Sets the geometry from a WKB string.
 After successful read the wkb argument will be at the position where the reading has stopped.
.. seealso:: fromWkt
 :rtype: bool
%End

    virtual bool fromWkt( const QString &wkt ) = 0;
%Docstring
 Sets the geometry from a WKT string.
.. seealso:: fromWkb
 :rtype: bool
%End


    virtual QByteArray asWkb() const = 0;
%Docstring
 Returns a WKB representation of the geometry.
.. seealso:: asWkt
.. seealso:: asGML2
.. seealso:: asGML3
.. seealso:: asJson()
.. versionadded:: 3.0
 :rtype: QByteArray
%End

    virtual QString asWkt( int precision = 17 ) const = 0;
%Docstring
 Returns a WKT representation of the geometry.
 \param precision number of decimal places for coordinates
.. seealso:: asWkb()
.. seealso:: asGml2()
.. seealso:: asGml3()
.. seealso:: asJson()
 :rtype: str
%End

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const = 0;
%Docstring
 Returns a GML2 representation of the geometry.
 \param doc DOM document
 \param precision number of decimal places for coordinates
 \param ns XML namespace
.. seealso:: asWkb()
.. seealso:: asWkt()
.. seealso:: asGml3()
.. seealso:: asJson()
 :rtype: QDomElement
%End

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const = 0;
%Docstring
 Returns a GML3 representation of the geometry.
 \param doc DOM document
 \param precision number of decimal places for coordinates
 \param ns XML namespace
.. seealso:: asWkb()
.. seealso:: asWkt()
.. seealso:: asGml2()
.. seealso:: asJson()
 :rtype: QDomElement
%End

    virtual QString asJson( int precision = 17 ) const = 0;
%Docstring
 Returns a GeoJSON representation of the geometry.
 \param precision number of decimal places for coordinates
.. seealso:: asWkb()
.. seealso:: asWkt()
.. seealso:: asGml2()
.. seealso:: asGml3()
 :rtype: str
%End


    virtual void transform( const QgsCoordinateTransform &ct,
                            QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                            bool transformZ = false ) = 0;
%Docstring
 Transforms the geometry using a coordinate transform
 \param ct coordinate transform
 \param d transformation direction
 \param transformZ set to true to also transform z coordinates. This requires that
 the z coordinates in the geometry represent height relative to the vertical datum
 of the source CRS (generally ellipsoidal heights) and are expressed in its vertical
 units (generally meters). If false, then z coordinates will not be changed by the
 transform.
%End

    virtual void transform( const QTransform &t ) = 0;
%Docstring
 Transforms the geometry using a QTransform object
 \param t QTransform transformation
%End

    virtual void draw( QPainter &p ) const = 0;
%Docstring
 Draws the geometry using the specified QPainter.
 \param p destination QPainter
%End

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const = 0;
%Docstring
 Returns the vertex number corresponding to a vertex ``id``.

 The vertex numbers start at 0, so a return value of 0 corresponds
 to the first vertex.

 Returns -1 if a corresponding vertex could not be found.

.. versionadded:: 3.0
 :rtype: int
%End

    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex /Out/ ) const = 0;
%Docstring
 Returns next vertex id and coordinates
 \param id initial value should be the starting vertex id. The next vertex id will be stored
 in this variable if found.
 \param vertex container for found node
 :return: false if at end
 :rtype: bool
%End

    virtual void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex /Out/, QgsVertexId &nextVertex /Out/ ) const = 0;
%Docstring
 Returns the vertices adjacent to a specified ``vertex`` within a geometry.
.. versionadded:: 3.0
%End

    virtual QgsCoordinateSequence coordinateSequence() const = 0;
%Docstring
 Retrieves the sequence of geometries, rings and nodes.
 :return: coordinate sequence
 :rtype: QgsCoordinateSequence
%End

    virtual int nCoordinates() const;
%Docstring
 Returns the number of nodes contained in the geometry
 :rtype: int
%End

    virtual QgsPoint vertexAt( QgsVertexId id ) const = 0;
%Docstring
 Returns the point corresponding to a specified vertex id
 :rtype: QgsPoint
%End

    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/,
                                   QgsVertexId &vertexAfter /Out/,
                                   bool *leftOf /Out/ = 0, double epsilon = 4 * DBL_EPSILON ) const = 0;
%Docstring
 Searches for the closest segment of the geometry to a given point.
 \param pt specifies the point to find closest segment to
 \param segmentPt storage for the closest point within the geometry
 \param vertexAfter storage for the ID of the vertex at the end of the closest segment
 \param leftOf returns whether the point lies on the left side of the nearest segment (true if point is to left of segment,
 false if point is to right of segment)
 \param epsilon epsilon for segment snapping
 :return: squared distance to closest segment or negative value on error
 :rtype: float
%End


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) = 0;
%Docstring
 Inserts a vertex into the geometry
 \param position vertex id for position of inserted vertex
 \param vertex vertex to insert
 :return: true if insert was successful
.. seealso:: moveVertex
.. seealso:: deleteVertex
 :rtype: bool
%End

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) = 0;
%Docstring
 Moves a vertex within the geometry
 \param position vertex id for vertex to move
 \param newPos new position of vertex
 :return: true if move was successful
.. seealso:: insertVertex
.. seealso:: deleteVertex
 :rtype: bool
%End

    virtual bool deleteVertex( QgsVertexId position ) = 0;
%Docstring
 Deletes a vertex within the geometry
 \param position vertex id for vertex to delete
 :return: true if delete was successful
.. seealso:: insertVertex
.. seealso:: moveVertex
 :rtype: bool
%End

    virtual double length() const;
%Docstring
 Returns the length of the geometry.
.. seealso:: area()
.. seealso:: perimeter()
 :rtype: float
%End

    virtual double perimeter() const;
%Docstring
 Returns the perimeter of the geometry.
.. seealso:: area()
.. seealso:: length()
 :rtype: float
%End

    virtual double area() const;
%Docstring
 Returns the area of the geometry.
.. seealso:: length()
.. seealso:: perimeter()
 :rtype: float
%End

    virtual double segmentLength( QgsVertexId startVertex ) const = 0;
%Docstring
 Returns the length of the segment of the geometry which begins at ``startVertex``.
.. versionadded:: 3.0
 :rtype: float
%End

    virtual QgsPoint centroid() const;
%Docstring
Returns the centroid of the geometry
 :rtype: QgsPoint
%End

    virtual bool isEmpty() const;
%Docstring
 Returns true if the geometry is empty
 :rtype: bool
%End

    virtual bool hasCurvedSegments() const;
%Docstring
 Returns true if the geometry contains curved segments
 :rtype: bool
%End

    virtual QgsAbstractGeometry *segmentize( double tolerance = M_PI / 180., SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;
%Docstring
 Returns a version of the geometry without curves. Caller takes ownership of
 the returned geometry.
 \param tolerance segmentation tolerance
 \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
 :rtype: QgsAbstractGeometry
%End

    virtual QgsAbstractGeometry *toCurveType() const = 0 /Factory/;
%Docstring
 Returns the geometry converted to the more generic curve type.
 E.g. QgsLineString -> QgsCompoundCurve, QgsPolygon -> QgsCurvePolygon,
 QgsMultiLineString -> QgsMultiCurve, QgsMultiPolygon -> QgsMultiSurface
 :return: the converted geometry. Caller takes ownership
 :rtype: QgsAbstractGeometry
%End

    virtual QgsAbstractGeometry *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const = 0 /Factory/;
%Docstring
 Makes a new geometry with all the points or vertices snapped to the closest point of the grid.
 Ownership is transferred to the caller.

 If the gridified geometry could not be calculated a None will be returned.
 It may generate an invalid geometry (in some corner cases).
 It can also be thought as rounding the edges and it may be useful for removing errors.
 Example:
 \code
 geometry->snappedToGrid(1, 1);
 \endcode
 In this case we use a 2D grid of 1x1 to gridify.
 In this case, it can be thought like rounding the x and y of all the points/vertices to full units (remove all decimals).
 \param hSpacing Horizontal spacing of the grid (x axis). 0 to disable.
 \param vSpacing Vertical spacing of the grid (y axis). 0 to disable.
 \param dSpacing Depth spacing of the grid (z axis). 0 (default) to disable.
 \param mSpacing Custom dimension spacing of the grid (m axis). 0 (default) to disable.
.. versionadded:: 3.0
 :rtype: QgsAbstractGeometry
%End

    virtual double vertexAngle( QgsVertexId vertex ) const = 0;
%Docstring
 Returns approximate angle at a vertex. This is usually the average angle between adjacent
 segments, and can be pictured as the orientation of a line following the curvature of the
 geometry at the specified vertex.
 \param vertex the vertex id
 :return: rotation in radians, clockwise from north
 :rtype: float
%End

    virtual int vertexCount( int part = 0, int ring = 0 ) const = 0;
%Docstring
 Returns the number of vertices of which this geometry is built.
 :rtype: int
%End

    virtual int ringCount( int part = 0 ) const = 0;
%Docstring
 Returns the number of rings of which this geometry is built.
 :rtype: int
%End

    virtual int partCount() const = 0;
%Docstring
 Returns count of parts contained in the geometry.
.. seealso:: vertexCount
.. seealso:: ringCount
 :rtype: int
%End

    virtual bool addZValue( double zValue = 0 ) = 0;
%Docstring
 Adds a z-dimension to the geometry, initialized to a preset value.
 \param zValue initial z-value for all nodes
 :return: true on success
.. versionadded:: 2.12
.. seealso:: dropZValue()
.. seealso:: addMValue()
 :rtype: bool
%End

    virtual bool addMValue( double mValue = 0 ) = 0;
%Docstring
 Adds a measure to the geometry, initialized to a preset value.
 \param mValue initial m-value for all nodes
 :return: true on success
.. versionadded:: 2.12
.. seealso:: dropMValue()
.. seealso:: addZValue()
 :rtype: bool
%End

    virtual bool dropZValue() = 0;
%Docstring
 Drops any z-dimensions which exist in the geometry.
 :return: true if Z values were present and have been removed
.. seealso:: addZValue()
.. seealso:: dropMValue()
.. versionadded:: 2.14
 :rtype: bool
%End

    virtual bool dropMValue() = 0;
%Docstring
 Drops any measure values which exist in the geometry.
 :return: true if m-values were present and have been removed
.. seealso:: addMValue()
.. seealso:: dropZValue()
.. versionadded:: 2.14
 :rtype: bool
%End

    virtual bool convertTo( QgsWkbTypes::Type type );
%Docstring
 Converts the geometry to a specified type.
 :return: true if conversion was successful
.. versionadded:: 2.14
 :rtype: bool
%End


    QgsVertexIterator vertices() const;
%Docstring
 Returns Java-style iterator for traversal of vertices of the geometry
.. versionadded:: 3.0
 :rtype: QgsVertexIterator
%End

  protected:

    virtual QgsAbstractGeometry *createEmptyWithSameType() const = 0 /Factory/;
%Docstring
 Creates a new geometry with the same class and same WKB type as the original and transfers ownership.
 To create it, the geometry is default constructed and then the WKB is changed.
.. seealso:: clone()
.. versionadded:: 3.0
.. note::

   Not available in Python bindings
 :rtype: QgsAbstractGeometry
%End

    virtual bool hasChildGeometries() const;
%Docstring
 Returns whether the geometry has any child geometries (false for point / curve, true otherwise)
.. note::

   used for vertex_iterator implementation
.. versionadded:: 3.0
 :rtype: bool
%End

    virtual int childCount() const;
%Docstring
 Returns number of child geometries (for geometries with child geometries) or child points (for geometries without child geometries - i.e. curve / point)
.. note::

   used for vertex_iterator implementation
.. versionadded:: 3.0
 :rtype: int
%End

    virtual QgsAbstractGeometry *childGeometry( int index ) const;
%Docstring
 Returns pointer to child geometry (for geometries with child geometries - i.e. geom. collection / polygon)
.. note::

   used for vertex_iterator implementation
.. versionadded:: 3.0
 :rtype: QgsAbstractGeometry
%End

    virtual QgsPoint childPoint( int index ) const;
%Docstring
 Returns point at index (for geometries without child geometries - i.e. curve / point)
.. note::

   used for vertex_iterator implementation
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

  protected:

    void setZMTypeFromSubGeometry( const QgsAbstractGeometry *subggeom, QgsWkbTypes::Type baseGeomType );
%Docstring
 Updates the geometry type based on whether sub geometries contain z or m values.
%End

    virtual QgsRectangle calculateBoundingBox() const;
%Docstring
 Default calculator for the minimal bounding box for the geometry. Derived classes should override this method
 if a more efficient bounding box calculation is available.
 :rtype: QgsRectangle
%End

    virtual void clearCache() const;
%Docstring
 Clears any cached parameters associated with the geometry, e.g., bounding boxes
%End

};


struct QgsVertexId
{
  enum VertexType
  {
    SegmentVertex,
    CurveVertex
  };

  explicit QgsVertexId( int _part = -1, int _ring = -1, int _vertex = -1, VertexType _type = SegmentVertex );

  bool isValid() const;
%Docstring
 Returns true if the vertex id is valid
 :rtype: bool
%End

  bool operator==( QgsVertexId other ) const;
  bool operator!=( QgsVertexId other ) const;
%Docstring
 :rtype: bool
%End
  bool partEqual( QgsVertexId o ) const;
%Docstring
 :rtype: bool
%End
  bool ringEqual( QgsVertexId o ) const;
%Docstring
 :rtype: bool
%End
  bool vertexEqual( QgsVertexId o ) const;
%Docstring
 :rtype: bool
%End
  bool isValid( const QgsAbstractGeometry *geom ) const;
%Docstring
 :rtype: bool
%End

  int part;
  int ring;
  int vertex;
  VertexType type;
};



class QgsVertexIterator
{
%Docstring
 Java-style iterator for traversal of vertices of a geometry
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractgeometry.h"
%End
  public:
    QgsVertexIterator();
%Docstring
Constructor for QgsVertexIterator
%End

    QgsVertexIterator( const QgsAbstractGeometry *geometry );
%Docstring
Constructs iterator for the given geometry
%End

    bool hasNext() const;
%Docstring
Find out whether there are more vertices
 :rtype: bool
%End

    QgsPoint next();
%Docstring
Return next vertex of the geometry (undefined behavior if hasNext() returns false before calling next())
 :rtype: QgsPoint
%End

    QgsVertexIterator *__iter__();
%Docstring
 :rtype: QgsVertexIterator
%End
%MethodCode
    sipRes = sipCpp;
%End

    SIP_PYOBJECT __next__();
%MethodCode
    if ( sipCpp->hasNext() )
      sipRes = sipConvertFromType( new QgsPoint( sipCpp->next() ), sipType_QgsPoint, Py_None );
    else
      PyErr_SetString( PyExc_StopIteration, "" );
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsabstractgeometry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
