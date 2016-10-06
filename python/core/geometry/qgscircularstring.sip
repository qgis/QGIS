class QgsCircularString: public QgsCurve
{
%TypeHeaderCode
#include <qgscircularstring.h>
%End

  public:
    QgsCircularString();
    ~QgsCircularString();

    virtual bool operator==( const QgsCurve& other ) const;
    virtual bool operator!=( const QgsCurve& other ) const;

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsCircularString* clone() const;
    virtual void clear();

    virtual bool fromWkb( QgsConstWkbPtr wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    int numPoints() const;

    /** Returns the point at index i within the circular string.
     */
    QgsPointV2 pointN( int i ) const;

    /**
     * @copydoc QgsCurve::points()
     */
    void points( QList<QgsPointV2>& pts ) const;

    /** Sets the circular string's points
     */
    void setPoints( const QList<QgsPointV2>& points );

    /**
     * @copydoc QgsAbstractGeometry::length()
     */
    virtual double length() const;

    /**
     * @copydoc QgsCurve::startPoint()
     */
    virtual QgsPointV2 startPoint() const;
    /**
     * @copydoc QgsCurve::endPoint()
     */
    virtual QgsPointV2 endPoint() const;
     /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineString* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const;

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    void transform( const QTransform& t );
    void addToPainterPath( QPainterPath& path ) const;

    /**
     * @copydoc QgsCurve::drawAsPolygon()
     */
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos );
    virtual bool deleteVertex( QgsVertexId position );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    /**
     * @copydoc QgsCurve::pointAt()
     */
    bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const;

    /**
     * @copydoc QgsCurve::sumUpArea()
     */
    void sumUpArea( double& sum ) const;

    /**
     * @copydoc QgsAbstractGeometry::hasCurvedSegments()
     */
    bool hasCurvedSegments() const;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        @param vertex the vertex id
        @return rotation in radians, clockwise from north*/
    double vertexAngle( QgsVertexId vertex ) const;

    virtual QgsCircularString* reversed() const /Factory/;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();
    virtual bool dropMValue();

    virtual double xAt( int index ) const;
    virtual double yAt( int index ) const;

  protected:

    virtual QgsRectangle calculateBoundingBox() const;

  private:
    //helper methods for curveToLine
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
};
