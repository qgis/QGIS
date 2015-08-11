class QgsCurvePolygonV2: public QgsSurfaceV2
{
%TypeHeaderCode
#include <qgscurvepolygonv2.h>
%End

  public:
    QgsCurvePolygonV2();
    QgsCurvePolygonV2( const QgsCurvePolygonV2& p );
    //QgsCurvePolygonV2& operator=( const QgsCurvePolygonV2& p );
    ~QgsCurvePolygonV2();

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsAbstractGeometryV2* clone() const;
    void clear();


    virtual QgsRectangle calculateBoundingBox() const;
    virtual bool fromWkb( const unsigned char* wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //surface interface
    virtual double area() const;
    virtual double length() const;
    QgsPointV2 centroid() const;
    QgsPointV2 pointOnSurface() const;
    QgsPolygonV2* surfaceToPolygon() const;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    virtual QgsPolygonV2* toPolygon() const;

    /** Sets exterior ring (takes ownership)*/
    void setExteriorRing( QgsCurveV2* ring /Transfer/ );
    /** Sets interior rings (takes ownership)*/
    void setInteriorRings( QList<QgsCurveV2*> rings );
    void addInteriorRing( QgsCurveV2* ring /Transfer/ );
    /** Removes ring. Exterior ring is 0, first interior ring 1, ...*/
    bool removeInteriorRing( int nr );

    virtual void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    bool hasCurvedSegments() const;
    QgsAbstractGeometryV2* segmentize() const /Factory/;
};
