/** \ingroup core
 * \class QgsPointV2
 * \brief Point geometry type, with support for z-dimension and m-values.
 * \note added in QGIS 2.10
 */
class QgsPointV2: public QgsAbstractGeometry
{
%TypeHeaderCode
#include <qgspointv2.h>
%End

  public:

    /** Construct a 2 dimensional point with an initial x and y coordinate.
     * @param x x-coordinate of point
     * @param y y-coordinate of point
     */
    QgsPointV2( double x = 0.0, double y = 0.0 );

    /** Construct a QgsPointV2 from a QgsPoint object
     */
    explicit QgsPointV2( const QgsPoint& p );

    /** Construct a QgsPointV2 from a QPointF
     */
    explicit QgsPointV2( QPointF p );

    /** Construct a point with a specified type (e.g., PointZ, PointM) and initial x, y, z, and m values.
     * @param type point type
     * @param x x-coordinate of point
     * @param y y-coordinate of point
     * @param z z-coordinate of point, for PointZ or PointZM types
     * @param m m-value of point, for PointM or PointZM types
     */
    QgsPointV2( QgsWkbTypes::Type type, double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0 );
%MethodCode
  if ( QgsWkbTypes::flatType( a0 ) != QgsWkbTypes::Point )
  {
    PyErr_SetString(PyExc_ValueError,
            QString( "%1 is not a valid WKB type for point geometries" ).arg( QgsWkbTypes::displayString( a0 ) ).toUtf8().constData() );
    sipIsErr = 1;
  }
  else
  {
    sipCpp = new sipQgsPointV2( a0, a1, a2, a3, a4 );
  }
%End

    bool operator==( const QgsPointV2& pt ) const;
    bool operator!=( const QgsPointV2& pt ) const;

    /** Returns the point's x-coordinate.
     * @see setX()
     * @see rx()
     */
    double x() const;

    /** Returns the point's y-coordinate.
     * @see setY()
     * @see ry()
     */
    double y() const;

    /** Returns the point's z-coordinate.
     * @see setZ()
     * @see rz()
     */
    double z() const;

    /** Returns the point's m value.
     * @see setM()
     * @see rm()
     */
    double m() const;

    /** Returns a reference to the x-coordinate of this point.
     * Using a reference makes it possible to directly manipulate x in place.
     * @see x()
     * @see setX()
     * @note not available in Python bindings
     */
    //double &rx();

    /** Returns a reference to the y-coordinate of this point.
     * Using a reference makes it possible to directly manipulate y in place.
     * @see y()
     * @see setY()
     * @note not available in Python bindings
     */
    //double &ry();

    /** Returns a reference to the z-coordinate of this point.
     * Using a reference makes it possible to directly manipulate z in place.
     * @see z()
     * @see setZ()
     * @note not available in Python bindings
     */
    //double &rz();

    /** Returns a reference to the m value of this point.
     * Using a reference makes it possible to directly manipulate m in place.
     * @see m()
     * @see setM()
     * @note not available in Python bindings
     */
    //double &rm()

    /** Sets the point's x-coordinate.
     * @see x()
     * @see rx()
     */
    void setX( double x );

    /** Sets the point's y-coordinate.
     * @see y()
     * @see ry()
     */
    void setY( double y );

    /** Sets the point's z-coordinate.
     * @note calling this will have no effect if the point does not contain a z-dimension. Use addZValue() to
     * add a z value and force the point to have a z dimension.
     * @see z()
     * @see rz()
     */
    void setZ( double z );

    /** Sets the point's m-value.
     * @note calling this will have no effect if the point does not contain a m-dimension. Use addMValue() to
     * add a m value and force the point to have an m dimension.
     * @see m()
     * @see rm()
     */
    void setM( double m );

    /** Returns the point as a QPointF.
     * @note added in QGIS 2.14
     */
    QPointF toQPointF() const;

    /**
     * Returns the distance between this point and a specified x, y coordinate. In certain
     * cases it may be more appropriate to call the faster distanceSquared() method,
     * e.g., when comparing distances.
     * @note added in QGIS 3.0
     * @see distanceSquared()
    */
    double distance( double x, double y ) const;

    /**
     * Returns the 2D distance between this point and another point. In certain
     * cases it may be more appropriate to call the faster distanceSquared() method,
     * e.g., when comparing distances.
     * @note added in QGIS 3.0
    */
    double distance( const QgsPointV2& other ) const;

    /**
     * Returns the squared distance between this point a specified x, y coordinate. Calling
     * this is faster than calling distance(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance() is not required.
     * @see distance()
     * @note added in QGIS 3.0
    */
    double distanceSquared( double x, double y ) const;

    /**
     * Returns the squared distance between this point another point. Calling
     * this is faster than calling distance(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance() is not required.
     * @see distance()
     * @note added in QGIS 3.0
    */
    double distanceSquared( const QgsPointV2& other ) const;

    /**
     * Calculates azimuth between this point and other one (clockwise in degree, starting from north)
     * @note added in QGIS 3.0
     */
    double azimuth( const QgsPointV2& other ) const;

    /** Returns a new point which correspond to this point projected by a specified distance
     * with specified angles (azimuth and inclination).
     * M value is preserved.
     * @param distance distance to project
     * @param azimuth angle to project in X Y, clockwise in degrees starting from north
     * @param inclination angle to project in Z (3D)
     * @return The point projected. If a 2D point is projected a 3D point will be returned except if
     *  inclination is 90. A 3D point is always returned if a 3D point is projected.
     * Example:
     * \code{.py}
     *   p = QgsPointV2( 1, 2 ) # 2D point
     *   pr = p.project ( 1, 0 )
     *   # pr is a 2D point: 'Point (1 3)'
     *   pr = p.project ( 1, 0, 90 )
     *   # pr is a 2D point: 'Point (1 3)'
     *   pr = p.project (1, 0, 0 )
     *   # pr is a 3D point: 'PointZ (1 2 1)'
     *   p = QgsPointV2( QgsWkbTypes.PointZ, 1, 2, 2 ) # 3D point
     *   pr = p.project ( 1, 0 )
     *   # pr is a 3D point: 'PointZ (1 3 2)'
     *   pr = p.project ( 1, 0, 90 )
     *   # pr is a 3D point: 'PointZ (1 3 2)'
     *   pr = p.project (1, 0, 0 )
     *   # pr is a 3D point: 'PointZ (1 2 3)'
     * \endcode
     * @note added in QGIS 3.0
     */
    QgsPointV2 project( double distance, double azimuth, double inclination = 90.0 ) const;

    /**
     * Calculates the vector obtained by subtracting a point from this point.
     * @note added in QGIS 3.0
     */
    QgsVector operator-( const QgsPointV2& p ) const;

    /**
     * Adds a vector to this point in place.
     * @note added in QGIS 3.0
     */
    QgsPointV2 &operator+=( QgsVector v );

    /**
     * Subtracts a vector from this point in place.
     * @note added in QGIS 3.0
     */
    QgsPointV2 &operator-=( QgsVector v );

    /**
     * Adds a vector to this point.
     * @note added in QGIS 3.0
     */
    QgsPointV2 operator+( QgsVector v ) const;

    /**
     * Subtracts a vector from this point.
     * @note added in QGIS 3.0
     */
    QgsPointV2 operator-( QgsVector v ) const;


    //implementation of inherited methods
    virtual QgsRectangle boundingBox() const;
    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsPointV2* clone() const /Factory/;
    void clear();
    bool isEmpty() const;
    virtual bool fromWkb( QgsConstWkbPtr& wkb );
    virtual bool fromWkt( const QString& wkt );
    QByteArray asWkb() const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;
    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    void transform( const QTransform& t );
    virtual QList< QList< QList< QgsPointV2 > > > coordinateSequence() const;
    virtual int nCoordinates() const;
    virtual QgsAbstractGeometry* boundary() const /Factory/;

    //low-level editing
    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos );
    virtual bool deleteVertex( QgsVertexId position );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    /** Angle undefined. Always returns 0.0
        @param vertex the vertex id
        @return 0.0*/
    double vertexAngle( QgsVertexId vertex ) const;

    virtual int vertexCount( int /*part*/ = 0, int ring = 0 ) const;
    virtual int ringCount( int /*part*/ = 0 ) const;
    virtual int partCount() const;
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );
    virtual bool dropZValue();
    virtual bool dropMValue();
    virtual bool convertTo( QgsWkbTypes::Type type );

};
