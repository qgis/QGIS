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
    virtual QgsAbstractGeometryV2* combine( const QList< QgsAbstractGeometryV2* >&, QString* errorMsg = 0 ) const = 0;
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

    /** Returns the Dimensional Extended 9 Intersection Model (DE-9IM) representation of the
     * relationship between the geometries.
     * @param geom geometry to relate to
     * @param errorMsg destination storage for any error message
     * @returns DE-9IM string for relationship, or an empty string if an error occurred
     * @note added in QGIS 2.12
     */
    virtual QString relate( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;

    /** Tests whether two geometries are related by a specified Dimensional Extended 9 Intersection Model (DE-9IM)
     * pattern.
     * @param geom geometry to relate to
     * @param pattern DE-9IM pattern for match
     * @param errorMsg destination storage for any error message
     * @returns true if geometry relationship matches with pattern
     * @note added in QGIS 2.14
     */
    virtual bool relatePattern( const QgsAbstractGeometryV2& geom, const QString& pattern, QString* errorMsg = 0 ) const = 0;

    virtual double area( QString* errorMsg = 0 ) const = 0;
    virtual double length( QString* errorMsg = 0 ) const = 0;
    virtual bool isValid( QString* errorMsg = 0 ) const = 0;
    virtual bool isEqual( const QgsAbstractGeometryV2& geom, QString* errorMsg = 0 ) const = 0;
    virtual bool isEmpty( QString* errorMsg = 0 ) const = 0;

    virtual int splitGeometry( const QgsLineStringV2& splitLine,
                               QList<QgsAbstractGeometryV2*>& newGeometries,
                               bool topological,
                               QList<QgsPointV2> &topologyTestPoints, QString* errorMsg = nullptr ) const;

    virtual QgsAbstractGeometryV2* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg = 0 ) const = 0;
};
