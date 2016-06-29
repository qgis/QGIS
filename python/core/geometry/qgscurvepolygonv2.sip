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
    virtual QgsCurvePolygonV2* clone() const;
    void clear();

    virtual bool fromWkb( QgsConstWkbPtr wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //surface interface
    virtual double area() const;
    virtual double perimeter() const;
    QgsPolygonV2* surfaceToPolygon() const;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    /** Returns a new polygon geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsPolygonV2* toPolygon( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const;

    /** Sets the exterior ring of the polygon. The CurvePolygon type will be updated to match the dimensionality
     * of the exterior ring. For instance, setting a 2D exterior ring on a 3D CurvePolygon will drop the z dimension
     * from the CurvePolygon and all interior rings.
     * @param ring new exterior ring. Ownership is transferred to the CurvePolygon.
     * @see setInteriorRings()
     * @see exteriorRing()
     */
    virtual void setExteriorRing( QgsCurveV2* ring /Transfer/ );

    /** Sets all interior rings (takes ownership)*/
    void setInteriorRings( const QList<QgsCurveV2*>& rings );
    /** Adds an interior ring to the geometry (takes ownership)*/
    virtual void addInteriorRing( QgsCurveV2* ring /Transfer/ );
    /** Removes ring. Exterior ring is 0, first interior ring 1, ...*/
    bool removeInteriorRing( int nr );

    virtual void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    void transform( const QTransform& t );

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos );
    virtual bool deleteVertex( QgsVertexId position );

    virtual QList< QList< QList< QgsPointV2 > > > coordinateSequence() const;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    bool hasCurvedSegments() const;
    /** Returns a geometry without curves. Caller takes ownership
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsAbstractGeometryV2* segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
     *  @param vertex the vertex id
     *  @return rotation in radians, clockwise from north
     */
    double vertexAngle( QgsVertexId vertex ) const;

    virtual int vertexCount( int /*part*/ = 0, int ring = 0 ) const;
    virtual int ringCount( int /*part*/ = 0 ) const;
    virtual int partCount() const;
    virtual QgsPointV2 vertexAt( const QgsVertexId& id ) const;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );
    virtual bool dropZValue();
    virtual bool dropMValue();

  protected:

    virtual QgsRectangle calculateBoundingBox() const;
};
