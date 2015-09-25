class QgsGeometryEngine
{
%TypeHeaderCode
#include <qgsgeometryengine.h>
%End

  public:
    QgsGeometryEngine( const QgsAbstractGeometryV2* geometry );
    virtual ~QgsGeometryEngine();

    virtual void geometryChanged() = 0;
    virtual void prepareGeometry() = 0;

    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QList< const QgsAbstractGeometryV2* >, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* buffer( double distance, int segments, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* simplify( double tolerance, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* interpolate( double distance, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* envelope( QString* errorMsg = 0 ) const = 0;
    virtual bool centroid( QgsPointV2& pt, QString* errorMsg = 0 ) const = 0;
    virtual bool pointOnSurface( QgsPointV2& pt, QString* errorMsg = 0 ) const = 0;
    virtual QgsAbstractGeometryV2* convexHull( QString* errorMsg = 0 ) const = 0;
    virtual double distance( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool intersects( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool disjoint( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual double area( QString* errorMsg = 0 ) const = 0;
    virtual double length( QString* errorMsg = 0 ) const = 0;
    virtual bool isValid( QString* errorMsg = 0 ) const = 0;
    virtual bool isEqual( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool isEmpty( QString* errorMsg = 0 ) const = 0;

    virtual QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = 0 ) const = 0;
};
