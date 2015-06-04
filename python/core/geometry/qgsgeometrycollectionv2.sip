class QgsGeometryCollectionV2: public QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgsgeometrycollectionv2.h>
%End

  public:
    QgsGeometryCollectionV2();
    QgsGeometryCollectionV2( const QgsGeometryCollectionV2& c );
    //QgsGeometryCollectionV2& operator=( const QgsGeometryCollectionV2& c );
    virtual ~QgsGeometryCollectionV2();

    int numGeometries() const;
    //const QgsAbstractGeometryV2* geometryN( int n ) const;
    QgsAbstractGeometryV2* geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    virtual int dimension() const;
    virtual QString geometryType() const;
    virtual void clear();

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g /Transfer/ );
    virtual bool removeGeometry( int nr );

    virtual void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );
    virtual void clip( const QgsRectangle& rect );
    virtual void draw( QPainter& p ) const;

    bool fromWkb( const unsigned char * wkb );
    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    virtual QgsRectangle calculateBoundingBox() const;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual double length() const;
    virtual double area() const;

    bool hasCurvedSegments() const;
};
