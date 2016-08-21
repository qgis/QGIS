class QgsCurve: public QgsAbstractGeometry
{
%TypeHeaderCode
#include <qgscurve.h>
%End
    public:
    QgsCurve();
    virtual ~QgsCurve();

    virtual bool operator==( const QgsCurve& other ) const = 0;
    virtual bool operator!=( const QgsCurve& other ) const = 0;

    virtual QgsCurve* clone() const = 0;

    /** Returns the starting point of the curve.
     * @see endPoint
     */
    virtual QgsPointV2 startPoint() const = 0;

    /** Returns the end point of the curve.
     * @see startPoint
     */
    virtual QgsPointV2 endPoint() const = 0;

    /** Returns true if the curve is closed.
     */
    virtual bool isClosed() const;

    /** Returns true if the curve is a ring.
     */
    virtual bool isRing() const;

    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve
     */
    virtual QgsLineString* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const = 0;

    /** Adds a curve to a painter path.
     */
    virtual void addToPainterPath( QPainterPath& path ) const = 0;

    /** Draws the curve as a polygon on the specified QPainter.
     * @param p destination QPainter
     */
    virtual void drawAsPolygon( QPainter& p ) const = 0;

    /** Returns a list of points within the curve.
     */
    virtual void points( QList<QgsPointV2>& pt ) const = 0;

    /** Returns the number of points in the curve.
     */
    virtual int numPoints() const = 0;

    /** Calculates the area of the curve. Derived classes should override this
     * to return the correct area of the curve.
     */
    virtual void sumUpArea( double& sum ) const = 0;

    virtual QList< QList< QList< QgsPointV2 > > > coordinateSequence() const;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    /** Returns the point and vertex id of a point within the curve.
     * @param node node number, where the first node is 0
     * @param point will be set to point at corresponding node in the curve
     * @param type will be set to the vertex type of the node
     * @returns true if node exists within the curve
     */
    virtual bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const = 0;

    /** Returns a reversed copy of the curve, where the direction of the curve has been flipped.
     * @note added in QGIS 2.14
     */
    virtual QgsCurve* reversed() const = 0 /Factory/;

    virtual QgsAbstractGeometry* boundary() const /Factory/;

    /** Returns a geometry without curves. Caller takes ownership
    * @param tolerance segmentation tolerance
    * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsCurve* segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

    virtual int vertexCount( int part = 0, int ring = 0 ) const;
    virtual int ringCount( int part = 0 ) const;
    virtual int partCount() const;
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const;

    virtual QgsRectangle boundingBox() const;

    /** Drops any Z dimensions which exist in the geometry.
     * @returns true if Z values were present and have been removed
     * @see dropMValue()
     * @note added in QGIS 2.14
     */
    virtual bool dropZValue() = 0;

    /** Drops any M values which exist in the geometry.
     * @returns true if M values were present and have been removed
     * @see dropZValue()
     * @note added in QGIS 2.14
     */
    virtual bool dropMValue() = 0;

    /** Returns the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns x-coordinate of node, or 0.0 if index is out of bounds
     * @see setXAt()
     */
    virtual double xAt( int index ) const = 0;

    /** Returns the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns y-coordinate of node, or 0.0 if index is out of bounds
     * @see setYAt()
     */
    virtual double yAt( int index ) const = 0;

    /** Returns a QPolygonF representing the line string.
     */
    QPolygonF asQPolygonF() const;

  protected:

    virtual void clearCache() const;

};
