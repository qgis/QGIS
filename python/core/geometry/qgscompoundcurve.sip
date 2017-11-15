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

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJson( int precision = 17 ) const;


    virtual double length() const;

    virtual QgsPoint startPoint() const;

    virtual QgsPoint endPoint() const;

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

    virtual QgsCompoundCurve *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const /Factory/;


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
 Adds a curve to the geometry (takes ownership)
%End

    void removeCurve( int i );
%Docstring
 Removes a curve from the geometry.
 \param i index of curve to remove
%End

    void addVertex( const QgsPoint &pt );
%Docstring
 Adds a vertex to the end of the geometry.
%End

    virtual void draw( QPainter &p ) const;

     virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    virtual void transform( const QTransform &t );

    virtual void addToPainterPath( QPainterPath &path ) const;

    virtual void drawAsPolygon( QPainter &p ) const;

    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );

    virtual bool deleteVertex( QgsVertexId position );

    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/, QgsVertexId &vertexAfter /Out/, bool *leftOf /Out/ = 0, double epsilon = 4 * DBL_EPSILON ) const;

    virtual bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const;

    virtual void sumUpArea( double &sum /Out/ ) const;


    void close();
%Docstring
Appends first point if not already closed.
%End

    virtual bool hasCurvedSegments() const;

    virtual double vertexAngle( QgsVertexId vertex ) const;

    virtual double segmentLength( QgsVertexId startVertex ) const;

    virtual QgsCompoundCurve *reversed() const /Factory/;


    virtual bool addZValue( double zValue = 0 );

    virtual bool addMValue( double mValue = 0 );


    virtual bool dropZValue();

    virtual bool dropMValue();


    virtual double xAt( int index ) const;

    virtual double yAt( int index ) const;


  protected:

    virtual QgsRectangle calculateBoundingBox() const;

    virtual QgsCompoundCurve *createEmptyWithSameType() const /Factory/;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscompoundcurve.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
