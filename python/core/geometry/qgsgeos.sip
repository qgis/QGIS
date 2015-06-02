class QgsGeos: public QgsGeometryEngine
{
%TypeHeaderCode
#include <qgsgeos.h>
%End

  public:
    QgsGeos( const QgsAbstractGeometryV2* geometry );
    ~QgsGeos();

    /**Removes caches*/
    void geometryChanged();
    void prepareGeometry();

    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const ;
    QgsAbstractGeometryV2* combine( const QList< const QgsAbstractGeometryV2* > ) const;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* buffer( double distance, int segments ) const;
    QgsAbstractGeometryV2* simplify( double tolerance ) const;
    QgsAbstractGeometryV2* interpolate( double distance ) const;
    bool centroid( QgsPointV2& pt ) const;
    bool pointOnSurface( QgsPointV2& pt ) const;
    QgsAbstractGeometryV2* convexHull() const;
    double distance( const QgsAbstractGeometryV2& geom ) const;
    bool intersects( const QgsAbstractGeometryV2& geom ) const;
    bool touches( const QgsAbstractGeometryV2& geom ) const;
    bool crosses( const QgsAbstractGeometryV2& geom ) const;
    bool within( const QgsAbstractGeometryV2& geom ) const;
    bool overlaps( const QgsAbstractGeometryV2& geom ) const;
    bool contains( const QgsAbstractGeometryV2& geom ) const;
    bool disjoint( const QgsAbstractGeometryV2& geom ) const;
    double area() const;
    double length() const;
    bool isValid() const;
    bool isEqual( const QgsAbstractGeometryV2& geom ) const;
    bool isEmpty() const;

    QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const;
    QgsAbstractGeometryV2* reshapeGeometry( const QgsLineStringV2& reshapeWithLine, int* errorCode ) const;
};
