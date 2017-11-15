/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgspoint.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsPoint: QgsAbstractGeometry
{
%Docstring
 Point geometry type, with support for z-dimension and m-values.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspoint.h"
%End
  public:

    QgsPoint( double x = 0.0, double y = 0.0, SIP_PYOBJECT z = Py_None, SIP_PYOBJECT m = Py_None, QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown ) [( double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0, QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown )];
%Docstring
 Construct a point with the provided initial coordinate values.

 If ``wkbType`` is set to `QgsWkbTypes.Point`, `QgsWkbTypes.PointZ`, `QgsWkbTypes.PointM` or `QgsWkbTypes.PointZM`
 the type will be set accordingly. If it is left to the default `QgsWkbTypes.Unknown`, the type will be set
 based on the following rules:
 - If only x and y are specified, the type will be a 2D point.
 - If any or both of the Z and M are specified, the appropriate type will be created.

 \code{.py}
   pt = QgsPoint(43.4, 5.3)
   pt.asWkt() # Point(43.4 5.3)

   pt_z = QgsPoint(120, 343, 77)
   pt.asWkt() # PointZ(120 343 77)

   pt_m = QgsPoint(33, 88, m=5)
   pt_m.m() # 5
   pt_m.wkbType() # QgsWkbTypes.PointM

   pt = QgsPoint(30, 40, wkbType=QgsWkbTypes.PointZ)
   pt.z() # nan
   pt.wkbType() # QgsWkbTypes.PointZ
 \endcode
%End
%MethodCode
    double z;
    double m;

    if ( a2 == Py_None )
    {
      z = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      z = PyFloat_AsDouble( a2 );
    }

    if ( a3 == Py_None )
    {
      m = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      m = PyFloat_AsDouble( a3 );
    }

    sipCpp = new sipQgsPoint( a0, a1, z, m, a4 );
%End

    explicit QgsPoint( const QgsPointXY &p );
%Docstring
 Construct a QgsPoint from a QgsPointXY object
%End

    explicit QgsPoint( QPointF p );
%Docstring
 Construct a QgsPoint from a QPointF
%End


    bool operator==( const QgsPoint &pt ) const;
    bool operator!=( const QgsPoint &pt ) const;
%Docstring
 :rtype: bool
%End

    double x() const;
%Docstring
 Returns the point's x-coordinate.
.. seealso:: setX()
.. seealso:: rx()
 :rtype: float
%End

    double y() const;
%Docstring
 Returns the point's y-coordinate.
.. seealso:: setY()
.. seealso:: ry()
 :rtype: float
%End

    double z() const;
%Docstring
 Returns the point's z-coordinate.
.. seealso:: setZ()
.. seealso:: rz()
 :rtype: float
%End

    double m() const;
%Docstring
 Returns the point's m value.
.. seealso:: setM()
.. seealso:: rm()
 :rtype: float
%End





    void setX( double x );
%Docstring
 Sets the point's x-coordinate.
.. seealso:: x()
.. seealso:: rx()
%End

    void setY( double y );
%Docstring
 Sets the point's y-coordinate.
.. seealso:: y()
.. seealso:: ry()
%End

    void setZ( double z );
%Docstring
 Sets the point's z-coordinate.
.. note::

   calling this will have no effect if the point does not contain a z-dimension. Use addZValue() to
 add a z value and force the point to have a z dimension.
.. seealso:: z()
.. seealso:: rz()
%End

    void setM( double m );
%Docstring
 Sets the point's m-value.
.. note::

   calling this will have no effect if the point does not contain a m-dimension. Use addMValue() to
 add a m value and force the point to have an m dimension.
.. seealso:: m()
.. seealso:: rm()
%End

    QPointF toQPointF() const;
%Docstring
 Returns the point as a QPointF.
.. versionadded:: 2.14
 :rtype: QPointF
%End

    double distance( double x, double y ) const;
%Docstring
 Returns the distance between this point and a specified x, y coordinate. In certain
 cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
 when comparing distances.
.. versionadded:: 3.0
.. seealso:: distanceSquared()
 :rtype: float
%End

    double distance( const QgsPoint &other ) const;
%Docstring
 Returns the 2D distance between this point and another point. In certain
 cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
 when comparing distances.
.. versionadded:: 3.0
 :rtype: float
%End

    double distanceSquared( double x, double y ) const;
%Docstring
 Returns the squared distance between this point a specified x, y coordinate. Calling
 this is faster than calling distance(), and may be useful in use cases such as comparing
 distances where the extra expense of calling distance() is not required.
.. seealso:: distance()
.. versionadded:: 3.0
 :rtype: float
%End

    double distanceSquared( const QgsPoint &other ) const;
%Docstring
 Returns the squared distance between this point another point. Calling
 this is faster than calling distance(), and may be useful in use cases such as comparing
 distances where the extra expense of calling distance() is not required.
.. seealso:: distance()
.. versionadded:: 3.0
 :rtype: float
%End

    double distance3D( double x, double y, double z ) const;
%Docstring
 Returns the 3D distance between this point and a specified x, y, z coordinate. In certain
 cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
 when comparing distances.
.. versionadded:: 3.0
.. seealso:: distanceSquared()
 :rtype: float
%End

    double distance3D( const QgsPoint &other ) const;
%Docstring
 Returns the 3D distance between this point and another point. In certain
 cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
 when comparing distances.
.. versionadded:: 3.0
 :rtype: float
%End

    double distanceSquared3D( double x, double y, double z ) const;
%Docstring
 Returns the 3D squared distance between this point a specified x, y, z coordinate. Calling
 this is faster than calling distance(), and may be useful in use cases such as comparing
 distances where the extra expense of calling distance() is not required.
.. seealso:: distance()
.. versionadded:: 3.0
 :rtype: float
%End

    double distanceSquared3D( const QgsPoint &other ) const;
%Docstring
 Returns the 3D squared distance between this point another point. Calling
 this is faster than calling distance(), and may be useful in use cases such as comparing
 distances where the extra expense of calling distance() is not required.
.. seealso:: distance()
.. versionadded:: 3.0
 :rtype: float
%End

    double azimuth( const QgsPoint &other ) const;
%Docstring
 Calculates azimuth between this point and other one (clockwise in degree, starting from north)
.. versionadded:: 3.0
 :rtype: float
%End

    double inclination( const QgsPoint &other ) const;
%Docstring
 Calculates inclination between this point and other one (starting from zenith = 0 to nadir = 180. Horizon = 90)
 Returns 90.0 if the distance between this point and other one is equal to 0 (same point).
.. versionadded:: 3.0
 :rtype: float
%End

    QgsPoint project( double distance, double azimuth, double inclination = 90.0 ) const;
%Docstring
 Returns a new point which correspond to this point projected by a specified distance
 with specified angles (azimuth and inclination).
 M value is preserved.
 \param distance distance to project
 \param azimuth angle to project in X Y, clockwise in degrees starting from north
 \param inclination angle to project in Z (3D). If the point is 2D, the Z value is assumed to be 0.
 :return: The point projected. If a 2D point is projected a 3D point will be returned except if
  inclination is 90. A 3D point is always returned if a 3D point is projected.
 Example:
 \code{.py}
   p = QgsPoint( 1, 2 ) # 2D point
   pr = p.project ( 1, 0 )
   # pr is a 2D point: 'Point (1 3)'
   pr = p.project ( 1, 0, 90 )
   # pr is a 2D point: 'Point (1 3)'
   pr = p.project (1, 0, 0 )
   # pr is a 3D point: 'PointZ (1 2 1)'
   p = QgsPoint( QgsWkbTypes.PointZ, 1, 2, 2 ) # 3D point
   pr = p.project ( 1, 0 )
   # pr is a 3D point: 'PointZ (1 3 2)'
   pr = p.project ( 1, 0, 90 )
   # pr is a 3D point: 'PointZ (1 3 2)'
   pr = p.project (1, 0, 0 )
   # pr is a 3D point: 'PointZ (1 2 3)'
 \endcode
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

    QgsVector operator-( const QgsPoint &p ) const;
%Docstring
 Calculates the vector obtained by subtracting a point from this point.
.. versionadded:: 3.0
 :rtype: QgsVector
%End

    QgsPoint &operator+=( QgsVector v );
%Docstring
 Adds a vector to this point in place.
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

    QgsPoint &operator-=( QgsVector v );
%Docstring
 Subtracts a vector from this point in place.
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

    QgsPoint operator+( QgsVector v ) const;
%Docstring
 Adds a vector to this point.
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

    QgsPoint operator-( QgsVector v ) const;
%Docstring
 Subtracts a vector from this point.
.. versionadded:: 3.0
 :rtype: QgsPoint
%End

    virtual bool isEmpty() const;

    virtual QgsRectangle boundingBox() const;

    virtual QString geometryType() const;

    virtual int dimension() const;

    virtual QgsPoint *clone() const /Factory/;

    virtual QgsPoint *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const /Factory/;
    virtual void clear();

    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );

    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJson( int precision = 17 ) const;

    virtual void draw( QPainter &p ) const;

     virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    virtual void transform( const QTransform &t );

    virtual QgsCoordinateSequence coordinateSequence() const;

    virtual int nCoordinates() const;

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const;

    virtual QgsAbstractGeometry *boundary() const /Factory/;


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );

    virtual bool deleteVertex( QgsVertexId position );


    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/, QgsVertexId &vertexAfter /Out/, bool *leftOf /Out/ = 0, double epsilon = 4 * DBL_EPSILON ) const;

    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex /Out/ ) const;

    virtual void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex /Out/, QgsVertexId &nextVertex /Out/ ) const;


    virtual double vertexAngle( QgsVertexId vertex ) const;

%Docstring
 Angle undefined. Always returns 0.0
\param vertex the vertex id
:return: 0.0*
 :rtype: float
%End

    virtual int vertexCount( int /*part*/ = 0, int /*ring*/ = 0 ) const;

    virtual int ringCount( int /*part*/ = 0 ) const;

    virtual int partCount() const;

    virtual QgsPoint vertexAt( QgsVertexId /*id*/ ) const;

    virtual QgsPoint *toCurveType() const /Factory/;

    virtual double segmentLength( QgsVertexId startVertex ) const;


    virtual bool addZValue( double zValue = 0 );

    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();

    virtual bool dropMValue();

    virtual bool convertTo( QgsWkbTypes::Type type );



  protected:
    virtual QgsPoint *createEmptyWithSameType() const /Factory/;

    virtual int childCount() const;

    virtual QgsPoint childPoint( int index ) const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgspoint.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
