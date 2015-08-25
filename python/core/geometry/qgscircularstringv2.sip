class QgsCircularStringV2: public QgsCurveV2
{
%TypeHeaderCode
#include <qgscircularstringv2.h>
%End

  public:
    QgsCircularStringV2();
    ~QgsCircularStringV2();

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsAbstractGeometryV2* clone() const;
    virtual void clear();

    virtual QgsRectangle calculateBoundingBox() const;

    virtual bool fromWkb( const unsigned char * wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pts ) const;
    void setPoints( const QList<QgsPointV2>& points );


    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual QgsLineStringV2* curveToLine() const;

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform );
    void transform( const QTransform& t );
    //void clip( const QgsRectangle& rect );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;

    bool hasCurvedSegments() const;

  private:
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
};
