/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgslinestring.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsLineString: QgsCurve
{
%Docstring
Line string geometry type, with support for z-dimension and m-values.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgslinestring.h"
%End
  public:
    QgsLineString();

    QgsLineString( const QVector<QgsPoint> &points );
%Docstring
Construct a linestring from a vector of points.
Z and M type will be set based on the type of the first point
in the vector.

.. versionadded:: 3.0
%End

    QgsLineString( const QVector<double> &x, const QVector<double> &y,
                   const QVector<double> &z = QVector<double>(),
                   const QVector<double> &m = QVector<double>(), bool is25DType = false );
%Docstring
Construct a linestring from arrays of coordinates. If the z or m
arrays are non-empty then the resultant linestring will have
z and m types accordingly.
This constructor is more efficient then calling setPoints()
or repeatedly calling addVertex()

If the ``z`` vector is filled, then the geometry type will either
be a LineStringZ(M) or LineString25D depending on the ``is25DType``
argument. If ``is25DType`` is ``True`` (and the ``m`` vector is unfilled) then
the created Linestring will be a LineString25D type. Otherwise, the
LineString will be LineStringZ (or LineStringZM) type.

.. versionadded:: 3.0
%End

    QgsLineString( const QgsPoint &p1, const QgsPoint &p2 );
%Docstring
Constructs a linestring with a single segment from ``p1`` to ``p2``.

.. versionadded:: 3.2
%End

    QgsLineString( const QVector<QgsPointXY> &points );
%Docstring
Construct a linestring from list of points.
This constructor is more efficient then calling setPoints()
or repeatedly calling addVertex()

.. versionadded:: 3.0
%End

    explicit QgsLineString( const QgsLineSegment2D &segment );
%Docstring
Construct a linestring from a single 2d line segment.

.. versionadded:: 3.2
%End

    virtual bool equals( const QgsCurve &other ) const;



    SIP_PYOBJECT pointN( int i ) const /TypeHint="QgsPoint"/;
%Docstring
Returns the point at the specified index. An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      std::unique_ptr< QgsPoint > p;
      if ( a0 >= 0 )
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else // negative index, count backwards from end
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
      sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
    }
%End


    virtual double xAt( int index ) const;

%Docstring
Returns the x-coordinate of the specified node in the line string.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->xAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->xAt( count + a0 ) );
    }
%End


    virtual double yAt( int index ) const;

%Docstring
Returns the y-coordinate of the specified node in the line string.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->yAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->yAt( count + a0 ) );
    }
%End






    double zAt( int index ) const;
%Docstring
Returns the z-coordinate of the specified node in the line string.

An IndexError will be raised if no point with the specified index exists.

If the LineString does not have a z-dimension then ``nan`` will be returned.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->zAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->zAt( count + a0 ) );
    }
%End


    double mAt( int index ) const;
%Docstring
Returns the m-coordinate of the specified node in the line string.

An IndexError will be raised if no point with the specified index exists.

If the LineString does not have a m-dimension then ``nan`` will be returned.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->mAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->mAt( count + a0 ) );
    }
%End


    void setXAt( int index, double x );
%Docstring
Sets the x-coordinate of the specified node in the line string.
The corresponding node must already exist in line string.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. seealso:: :py:func:`xAt`
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setXAt( a0, a1 );
      else
        sipCpp->setXAt( count + a0, a1 );
    }
%End


    void setYAt( int index, double y );
%Docstring
Sets the y-coordinate of the specified node in the line string.
The corresponding node must already exist in line string.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. seealso:: :py:func:`yAt`
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setYAt( a0, a1 );
      else
        sipCpp->setYAt( count + a0, a1 );
    }
%End


    void setZAt( int index, double z );
%Docstring
Sets the z-coordinate of the specified node in the line string.
The corresponding node must already exist in line string and the line string must have z-dimension.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. seealso:: :py:func:`zAt`
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setZAt( a0, a1 );
      else
        sipCpp->setZAt( count + a0, a1 );
    }
%End


    void setMAt( int index, double m );
%Docstring
Sets the m-coordinate of the specified node in the line string.
The corresponding node must already exist in line string and the line string must have m-dimension.

An IndexError will be raised if no point with the specified index exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. seealso:: :py:func:`mAt`
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setMAt( a0, a1 );
      else
        sipCpp->setMAt( count + a0, a1 );
    }
%End

    void setPoints( const QgsPointSequence &points );
%Docstring
Resets the line string to match the specified list of points. The line string will
inherit the dimensionality of the first point in the list.

:param points: new points for line string. If empty, line string will be cleared.
%End

    void append( const QgsLineString *line );
%Docstring
Appends the contents of another line string to the end of this line string.

:param line: line to append. Ownership is not transferred.
%End

    void addVertex( const QgsPoint &pt );
%Docstring
Adds a new vertex to the end of the line string.

:param pt: vertex to add
%End

    void close();
%Docstring
Closes the line string by appending the first point to the end of the line, if it is not already closed.
%End

    virtual QgsCompoundCurve *toCurveType() const /Factory/;

%Docstring
Returns the geometry converted to the more generic curve type :py:class:`QgsCompoundCurve`

:return: the converted geometry. Caller takes ownership*
%End

    void extend( double startDistance, double endDistance );
%Docstring
Extends the line geometry by extrapolating out the start or end of the line
by a specified distance. Lines are extended using the bearing of the first or last
segment in the line.

.. versionadded:: 3.0
%End


    virtual QString geometryType() const;

    virtual int dimension() const;

    virtual QgsLineString *clone() const /Factory/;

    virtual void clear();

    virtual bool isEmpty() const;

    virtual QgsLineString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const /Factory/;

    virtual bool removeDuplicateNodes( double epsilon = 4 * DBL_EPSILON, bool useZValues = false );

    virtual QPolygonF asQPolygonF() const;


    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );


    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QString asJson( int precision = 17 ) const;


    virtual double length() const;

    virtual QgsPoint startPoint() const;

    virtual QgsPoint endPoint() const;


    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const  /Factory/;

%Docstring
Returns a new line string geometry corresponding to a segmentized approximation
of the curve.

:param tolerance: segmentation tolerance
:param toleranceType: maximum segmentation angle or maximum difference between approximation and curve*
%End

    virtual int numPoints() const;

    virtual int nCoordinates() const;

    virtual void points( QgsPointSequence &pt /Out/ ) const;


    virtual void draw( QPainter &p ) const;


    virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false )  throw( QgsCsException );

    virtual void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );


    virtual void addToPainterPath( QPainterPath &path ) const;

    virtual void drawAsPolygon( QPainter &p ) const;


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );

    virtual bool deleteVertex( QgsVertexId position );


    virtual QgsLineString *reversed() const /Factory/;

    virtual QgsPoint *interpolatePoint( double distance ) const /Factory/;

    virtual QgsLineString *curveSubstring( double startDistance, double endDistance ) const /Factory/;


    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/, QgsVertexId &vertexAfter /Out/, int *leftOf /Out/ = 0, double epsilon = 4 * DBL_EPSILON ) const;

    virtual bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const;


    virtual QgsPoint centroid() const;


    virtual void sumUpArea( double &sum /Out/ ) const;

    virtual double vertexAngle( QgsVertexId vertex ) const;

    virtual double segmentLength( QgsVertexId startVertex ) const;

    virtual bool addZValue( double zValue = 0 );

    virtual bool addMValue( double mValue = 0 );


    virtual bool dropZValue();

    virtual bool dropMValue();

    virtual void swapXy();


    virtual bool convertTo( QgsWkbTypes::Type type );



    virtual QgsLineString *createEmptyWithSameType() const /Factory/;


    SIP_PYOBJECT __repr__();
%MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsLineString: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
%End

    SIP_PYOBJECT __getitem__( int index ) /TypeHint="QgsPoint"/;
%Docstring
Returns the point at the specified ``index``. An IndexError will be raised if no point with the specified ``index`` exists.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. versionadded:: 3.6
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      std::unique_ptr< QgsPoint > p;
      if ( a0 >= 0 )
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
      sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
    }
%End

    void __setitem__( int index, const QgsPoint &point );
%Docstring
Sets the point at the specified ``index``. A point at the ``index`` must already exist or an IndexError will be raised.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. versionadded:: 3.6
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 < 0 )
        a0 = count + a0;
      sipCpp->setXAt( a0, a1->x() );
      sipCpp->setYAt( a0, a1->y() );
      if ( sipCpp->isMeasure() )
        sipCpp->setMAt( a0, a1->m() );
      if ( sipCpp->is3D() )
        sipCpp->setZAt( a0, a1->z() );
    }
%End


    void __delitem__( int index );
%Docstring
Deletes the vertex at the specified ``index``. A point at the ``index`` must already exist or an IndexError will be raised.

Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
corresponds to the last point in the line.

.. versionadded:: 3.6
%End
%MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 >= 0 && a0 < count )
      sipCpp->deleteVertex( QgsVertexId( -1, -1, a0 ) );
    else if ( a0 < 0 && a0 >= -count )
      sipCpp->deleteVertex( QgsVertexId( -1, -1, count + a0 ) );
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
%End


  protected:

    virtual QgsRectangle calculateBoundingBox() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgslinestring.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
