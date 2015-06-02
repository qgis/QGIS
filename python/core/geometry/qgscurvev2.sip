class QgsCurveV2: public QgsAbstractGeometryV2
{
%TypeHeaderCode
#include <qgscurvev2.h>
%End
    public:
    QgsCurveV2();
    virtual ~QgsCurveV2();
    virtual QgsPointV2 startPoint() const = 0;
    virtual QgsPointV2 endPoint() const = 0;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const = 0;

    virtual void addToPainterPath( QPainterPath& path ) const = 0;
    virtual void drawAsPolygon( QPainter& p ) const = 0;
    virtual void points( QList<QgsPointV2>& pt ) const = 0;
    virtual int numPoints() const = 0;

    virtual double area() const;
    virtual void sumUpArea( double& sum ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord /Out/ ) const;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;
    virtual bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const = 0;

    /**Returns a geometry without curves. Caller takes ownership*/
    QgsAbstractGeometryV2* segmentize() const /Factory/;
};
