/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircularstring.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsCircularString: QgsCurve
{
%Docstring
 Circular string geometry type
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgscircularstring.h"
%End
  public:
    QgsCircularString();

    virtual bool operator==( const QgsCurve &other ) const;

    virtual bool operator!=( const QgsCurve &other ) const;


    virtual QString geometryType() const;

    virtual int dimension() const;

    virtual QgsCircularString *clone() const /Factory/;

    virtual void clear();


    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );


    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJson( int precision = 17 ) const;


    virtual bool isEmpty() const;

    virtual int numPoints() const;


    QgsPoint pointN( int i ) const;
%Docstring
 Returns the point at index i within the circular string.
 :rtype: QgsPoint
%End

    virtual void points( QgsPointSequence &pts /Out/ ) const;


    void setPoints( const QgsPointSequence &points );
%Docstring
 Sets the circular string's points
%End

    virtual double length() const;

    virtual QgsPoint startPoint() const;

    virtual QgsPoint endPoint() const;

    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

    virtual QgsCircularString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const /Factory/;

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

    virtual bool hasCurvedSegments() const;

    virtual double vertexAngle( QgsVertexId vertex ) const;

    virtual double segmentLength( QgsVertexId startVertex ) const;

    virtual QgsCircularString *reversed() const  /Factory/;

    virtual bool addZValue( double zValue = 0 );

    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();

    virtual bool dropMValue();

    virtual double xAt( int index ) const;

    virtual double yAt( int index ) const;


  protected:
    virtual QgsCircularString *createEmptyWithSameType() const /Factory/;

    virtual QgsRectangle calculateBoundingBox() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscircularstring.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
