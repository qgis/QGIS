class QgsGeometryCache
{
%TypeHeaderCode
#include <qgsgeometrycache.h>
%End
  public:
    QgsGeometryCache();
    ~QgsGeometryCache();

    QgsGeometryMap& cachedGeometries();

    //! fetch geometry from cache, return true if successful
    bool geometry( QgsFeatureId fid, QgsGeometry& geometry );

    //! store a geometry in the cache
    void cacheGeometry( QgsFeatureId fid, const QgsGeometry& geom );

    //! get rid of the cached geometry
    void removeGeometry( QgsFeatureId fid );


    /** Deletes the geometries in mCachedGeometries */
    void deleteCachedGeometries();

    void setCachedGeometriesRect( const QgsRectangle& extent );
    const QgsRectangle& cachedGeometriesRect();

};
