class QgsCurveV2: public QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgscurvev2.h>
%End
    public:
    QgsCurveV2();
    virtual ~QgsCurveV2();

    virtual bool operator==( const QgsCurveV2& other ) const = 0;
    virtual bool operator!=( const QgsCurveV2& other ) const = 0;

    virtual QgsCurveV2* clone() const = 0;

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
    virtual QgsLineStringV2* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const = 0;

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
    virtual QgsCurveV2* reversed() const = 0 /Factory/;

    /** Returns a geometry without curves. Caller takes ownership
    * @param tolerance segmentation tolerance
    * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsCurveV2* segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

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

  protected:

    virtual void clearCache() const;

};
