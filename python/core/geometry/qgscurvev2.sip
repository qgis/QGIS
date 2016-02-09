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

    virtual QgsPointV2 startPoint() const = 0;
    virtual QgsPointV2 endPoint() const = 0;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const = 0;

    virtual void addToPainterPath( QPainterPath& path ) const = 0;
    virtual void drawAsPolygon( QPainter& p ) const = 0;
    virtual void points( QList<QgsPointV2>& pt ) const = 0;
    virtual int numPoints() const = 0;

    virtual void sumUpArea( double& sum ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;
    virtual bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const = 0;

    /** Returns a reversed copy of the curve, where the direction of the curve has been flipped.
     * @note added in QGIS 2.14
     */
    virtual QgsCurveV2* reversed() const = 0 /Factory/;

    /** Returns a geometry without curves. Caller takes ownership*/
    virtual QgsCurveV2* segmentize() const /Factory/;

    virtual int vertexCount(int part = 0, int ring = 0) const;
    virtual int ringCount(int part = 0) const;
    virtual int partCount() const;
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const;

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
};
