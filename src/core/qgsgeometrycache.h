#ifndef QGSGEOMETRYCACHE_H
#define QGSGEOMETRYCACHE_H

#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsrectangle.h"

#include <QMap>

class CORE_EXPORT QgsGeometryCache
{
  public:
    QgsGeometryCache();
    ~QgsGeometryCache();

    inline QgsGeometryMap& cachedGeometries() { return mCachedGeometries; }

    //! fetch geometry from cache, return true if successful
    bool geometry( QgsFeatureId fid, QgsGeometry& geometry );

    //! store a geometry in the cache
    void cacheGeometry( QgsFeatureId fid, const QgsGeometry& geom );

    //! get rid of the cached geometry
    void removeGeometry( QgsFeatureId fid ) { mCachedGeometries.remove( fid ); }


    /** Deletes the geometries in mCachedGeometries */
    void deleteCachedGeometries();

    void setCachedGeometriesRect( const QgsRectangle& extent ) { mCachedGeometriesRect = extent; }
    const QgsRectangle& cachedGeometriesRect() { return mCachedGeometriesRect; }

  protected:

    /** Cache of the committed geometries retrieved *for the current display* */
    QgsGeometryMap mCachedGeometries;

    /** Extent for which there are cached geometries */
    QgsRectangle mCachedGeometriesRect;

};

#endif // QGSGEOMETRYCACHE_H
