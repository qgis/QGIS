/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscompoundcurve.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsCompoundCurve: QgsCurve
{
%Docstring
 Compound curve geometry type
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgscompoundcurve.h"
%End
  public:
    QgsCompoundCurve();
    QgsCompoundCurve( const QgsCompoundCurve &curve );
    ~QgsCompoundCurve();

    virtual bool operator==( const QgsCurve &other ) const;
    virtual bool operator!=( const QgsCurve &other ) const;

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsCompoundCurve *clone() const /Factory/;
    virtual void clear();

    virtual bool fromWkb( QgsConstWkbPtr &wkb );
    virtual bool fromWkt( const QString &wkt );

    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;


    virtual double length() const;
%Docstring
curve interface
 :rtype: float
%End
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual void points( QgsPointSequence &pts /Out/ ) const;
    virtual int numPoints() const;
    virtual bool isEmpty() const;


    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;
%Docstring
 Returns a new line string geometry corresponding to a segmentized approximation
 of the curve.
 \param tolerance segmentation tolerance
 \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*
 :rtype: QgsLineString
%End

    int nCurves() const;
%Docstring
 Returns the number of curves in the geometry.
 :rtype: int
%End

    const QgsCurve *curveAt( int i ) const;
%Docstring
 Returns the curve at the specified index.
 :rtype: QgsCurve
%End

    void addCurve( QgsCurve *c /Transfer/ );
%Docstring
 Adds a curve to the geometr (takes ownership)
%End

    void removeCurve( int i );
%Docstring
 Removes a curve from the geometry.
 \param i index of curve to remove
%End

    void addVertex( const QgsPointV2 &pt );
%Docstring
 Adds a vertex to the end of the geometry.
%End

    virtual void draw( QPainter &p ) const;

     virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    virtual void transform( const QTransform &t );

    virtual void addToPainterPath( QPainterPath &path ) const;

    virtual void drawAsPolygon( QPainter &p ) const;


    virtual bool insertVertex( QgsVertexId position, const QgsPointV2 &vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2 &newPos );
    virtual bool deleteVertex( QgsVertexId position );

    virtual double closestSegment( const QgsPointV2 &pt, QgsPointV2 &segmentPt /Out/,
                                   QgsVertexId &vertexAfter /Out/, bool *leftOf /Out/,
                                   double epsilon ) const;

    virtual bool pointAt( int node, QgsPointV2 &point, QgsVertexId::VertexType &type ) const;


    virtual void sumUpArea( double &sum /Out/ ) const;


    void close();
%Docstring
Appends first point if not already closed.
%End

    virtual bool hasCurvedSegments() const;


    virtual double vertexAngle( QgsVertexId vertex ) const;

%Docstring
 Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
\param vertex the vertex id
:return: rotation in radians, clockwise from north*
 :rtype: float
%End

    virtual QgsCompoundCurve *reversed() const /Factory/;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();
    virtual bool dropMValue();

    virtual double xAt( int index ) const;

    virtual double yAt( int index ) const;


  protected:

    virtual QgsRectangle calculateBoundingBox() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscompoundcurve.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
