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
                   const QVector<double> &m = QVector<double>() );
%Docstring
Construct a linestring from arrays of coordinates. If the z or m
arrays are non-empty then the resultant linestring will have
z and m types accordingly.
This constructor is more efficient then calling setPoints()
or repeatedly calling addVertex()

.. versionadded:: 3.0
%End

    QgsLineString( const QVector<QgsPointXY> &points );
%Docstring
Construct a linestring from list of points.
This constructor is more efficient then calling setPoints()
or repeatedly calling addVertex()

.. versionadded:: 3.0
%End

    virtual bool equals( const QgsCurve &other ) const;


    QgsPoint pointN( int i ) const;
%Docstring
Returns the specified point from inside the line string.

:param i: index of point, starting at 0 for the first point
%End

    virtual double xAt( int index ) const;

    virtual double yAt( int index ) const;


    double zAt( int index ) const;
%Docstring
Returns the z-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0

:return: z-coordinate of node, or ``nan`` if index is out of bounds or the line
does not have a z dimension

.. seealso:: :py:func:`setZAt`
%End

    double mAt( int index ) const;
%Docstring
Returns the m value of the specified node in the line string.

:param index: index of node, where the first node in the line is 0

:return: m value of node, or ``nan`` if index is out of bounds or the line
does not have m values

.. seealso:: :py:func:`setMAt`
%End

    void setXAt( int index, double x );
%Docstring
Sets the x-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0. Corresponding
node must already exist in line string.
:param x: x-coordinate of node

.. seealso:: :py:func:`xAt`
%End

    void setYAt( int index, double y );
%Docstring
Sets the y-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0. Corresponding
node must already exist in line string.
:param y: y-coordinate of node

.. seealso:: :py:func:`yAt`
%End

    void setZAt( int index, double z );
%Docstring
Sets the z-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0. Corresponding
node must already exist in line string, and the line string must have z-dimension.
:param z: z-coordinate of node

.. seealso:: :py:func:`zAt`
%End

    void setMAt( int index, double m );
%Docstring
Sets the m value of the specified node in the line string.

:param index: index of node, where the first node in the line is 0. Corresponding
node must already exist in line string, and the line string must have m values.
:param m: m value of node

.. seealso:: :py:func:`mAt`
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


    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );


    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

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


     virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    virtual void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );


    virtual void addToPainterPath( QPainterPath &path ) const;

    virtual void drawAsPolygon( QPainter &p ) const;


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );

    virtual bool deleteVertex( QgsVertexId position );


    virtual QgsLineString *reversed() const /Factory/;


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


    virtual bool convertTo( QgsWkbTypes::Type type );


  protected:
    virtual QgsLineString *createEmptyWithSameType() const /Factory/;

    virtual QgsRectangle calculateBoundingBox() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgslinestring.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
