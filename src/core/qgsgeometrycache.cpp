#include "qgsgeometrycache.h"

#include "qgsvectorlayereditbuffer.h"

QgsGeometryCache::QgsGeometryCache()
{
}

QgsGeometryCache::~QgsGeometryCache()
{
  // Destroy any cached geometries and clear the references to them
  deleteCachedGeometries();
}

bool QgsGeometryCache::geometry( QgsFeatureId fid, QgsGeometry& geometry )
{
  // no need to check changed geometries because all changed geometries are also cached

  // first time this geometry has changed since last commit
  if ( !mCachedGeometries.contains( fid ) )
    return false;

  geometry = mCachedGeometries[fid];
  return true;
}

void QgsGeometryCache::cacheGeometry( QgsFeatureId fid, const QgsGeometry& geom )
{
  mCachedGeometries[fid] = geom;
}


void QgsGeometryCache::deleteCachedGeometries()
{
  // Destroy any cached geometries
  mCachedGeometries.clear();
  mCachedGeometriesRect = QgsRectangle();
}
