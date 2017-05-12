/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometrycollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGeometryCollection: QgsAbstractGeometry
{
%Docstring
 Geometry collection
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsgeometrycollection.h"
%End
  public:
    QgsGeometryCollection();
    QgsGeometryCollection( const QgsGeometryCollection &c );
    virtual ~QgsGeometryCollection();

    virtual QgsGeometryCollection *clone() const /Factory/;

    int numGeometries() const;
%Docstring
 Returns the number of geometries within the collection.
 :rtype: int
%End


    QgsAbstractGeometry *geometryN( int n );
%Docstring
 Returns a geometry from within the collection.
 \param n index of geometry to return
 :rtype: QgsAbstractGeometry
%End

    virtual bool isEmpty() const;

    virtual int dimension() const;
    virtual QString geometryType() const;
    virtual void clear();
    virtual QgsAbstractGeometry *boundary() const /Factory/;

    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns true in case of success.
 :rtype: bool
%End

    virtual bool insertGeometry( QgsAbstractGeometry *g, int index /Transfer/ );
%Docstring
 Inserts a geometry before a specified index and takes ownership. Returns true in case of success.
 \param g geometry to insert. Ownership is transferred to the collection.
 \param index position to insert geometry before
 :rtype: bool
%End

    virtual bool removeGeometry( int nr );
%Docstring
 Removes a geometry from the collection.
 \param nr index of geometry to remove
 :return: true if removal was successful.
 :rtype: bool
%End

    virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                            bool transformZ = false );
    virtual void transform( const QTransform &t );

    virtual void draw( QPainter &p ) const;

    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );
    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const;

    virtual QString asJSON( int precision = 17 ) const;


    virtual QgsRectangle boundingBox() const;

    virtual QgsCoordinateSequence coordinateSequence() const;
    virtual int nCoordinates() const;

    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/,
                                   QgsVertexId &vertexAfter /Out/, bool *leftOf /Out/,
                                   double epsilon ) const;
    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex /Out/ ) const;


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );
    virtual bool deleteVertex( QgsVertexId position );

    virtual double length() const;
    virtual double area() const;
    virtual double perimeter() const;

    virtual bool hasCurvedSegments() const;


    virtual QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

%Docstring
 Returns a geometry without curves. Caller takes ownership
 \param tolerance segmentation tolerance
 \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*
 :rtype: QgsAbstractGeometry
%End

    virtual double vertexAngle( QgsVertexId vertex ) const;

%Docstring
 Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
 \param vertex the vertex id
 :return: rotation in radians, clockwise from north
 :rtype: float
%End

    virtual int vertexCount( int part = 0, int ring = 0 ) const;
    virtual int ringCount( int part = 0 ) const;
    virtual int partCount() const;
    virtual QgsPoint vertexAt( QgsVertexId id ) const;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );
    virtual bool dropZValue();
    virtual bool dropMValue();


  protected:

    virtual bool wktOmitChildType() const;
%Docstring
 Returns whether child type names are omitted from Wkt representations of the collection
.. versionadded:: 2.12
 :rtype: bool
%End

    bool fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometry *> &subtypes, const QString &defaultChildWkbType = QString() );
%Docstring
 Reads a collection from a WKT string.
 :rtype: bool
%End

    virtual QgsRectangle calculateBoundingBox() const;
    virtual void clearCache() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometrycollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
