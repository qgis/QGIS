#ifndef QGSVECTORLAYERCACHE_H
#define QGSVECTORLAYERCACHE_H

#include "qgsfeature.h"

#include "qgsvectorlayer.h"

class QgsVectorLayerCache
{
public:
  QgsVectorLayerCache(QgsVectorLayer* layer);
  ~QgsVectorLayerCache();

  inline QgsGeometryMap& cachedGeometries() { return mCachedGeometries; }

  //! fetch geometry from cache, return true if successful
  bool geometry(QgsFeatureId fid, QgsGeometry& geometry);

  //! store a geometry in the cache
  void cacheGeometry( QgsFeatureId fid, const QgsGeometry& geom );

  //! get rid of the cached geometry
  void removeGeometry( QgsFeatureId fid ) { mCachedGeometries.remove(fid); }


  /** Deletes the geometries in mCachedGeometries */
  void deleteCachedGeometries();

  void setCachedGeometriesRect( const QgsRectangle& extent ) { mCachedGeometriesRect = extent; }
  const QgsRectangle& cachedGeometriesRect() { return mCachedGeometriesRect; }

protected:

  inline QgsVectorLayerEditBuffer* editBuffer() { return L->editBuffer(); }

  QgsVectorLayer* L;

  /** cache of the committed geometries retrieved *for the current display* */
  QgsGeometryMap mCachedGeometries;

  /** extent for which there are cached geometries */
  QgsRectangle mCachedGeometriesRect;

};

#endif // QGSVECTORLAYERCACHE_H
