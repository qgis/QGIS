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

    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QList< const QgsAbstractGeometryV2* > ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* buffer( double distance, int segments ) const = 0;
    virtual QgsAbstractGeometryV2* simplify( double tolerance ) const = 0;
    virtual QgsAbstractGeometryV2* interpolate( double distance ) const = 0;
    virtual bool centroid( QgsPointV2& pt ) const = 0;
    virtual bool pointOnSurface( QgsPointV2& pt ) const = 0;
    virtual QgsAbstractGeometryV2* convexHull() const = 0;
    virtual double distance( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool intersects( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool disjoint( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual double area() const = 0;
    virtual double length() const = 0;
    virtual bool isValid() const = 0;
    virtual bool isEqual( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool isEmpty() const = 0;

    virtual QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const = 0;
};
