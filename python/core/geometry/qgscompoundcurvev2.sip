class QgsCompoundCurveV2: public QgsCurveV2
{
%TypeHeaderCode
#include <qgscompoundcurvev2.h>
%End

  public:
    QgsCompoundCurveV2();
    QgsCompoundCurveV2( const QgsCompoundCurveV2& curve );
    //QgsCompoundCurveV2 &operator=( const QgsCompoundCurveV2 & curve );
    ~QgsCompoundCurveV2();

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsAbstractGeometryV2* clone() const;
    virtual void clear();

    virtual QgsRectangle calculateBoundingBox() const;

    virtual bool fromWkb( const unsigned char* wkb );
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
    virtual void points( QList<QgsPointV2>& pts ) const;
    virtual int numPoints() const;
    virtual QgsLineStringV2* curveToLine() const;
    int nCurves() const;
    const QgsCurveV2* curveAt( int i ) const;

    /** Adds curve (takes ownership)*/
    void addCurve( QgsCurveV2* c /Transfer/ );
    void removeCurve( int i );
    void addVertex( const QgsPointV2& pt );
    /** Returns closed ring based on curve (connects to start point if not already done)*/
    void close();

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;

    bool hasCurvedSegments() const;
};
