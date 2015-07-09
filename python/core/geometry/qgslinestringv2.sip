class QgsLineStringV2: public QgsCurveV2
{
%TypeHeaderCode
#include <qgslinestringv2.h>
%End

  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsAbstractGeometryV2* clone() const;
    virtual void clear();

    virtual bool fromWkb( const unsigned char* wkb );
    //void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual QgsLineStringV2* curveToLine() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pt ) const;

    void setPoints( const QList<QgsPointV2>& points );
    void append( const QgsLineStringV2* line );

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );

    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    const QPolygonF& qPolygonF() const;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );
    void addVertex( const QgsPointV2& pt );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;
};
