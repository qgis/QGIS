#include "qgsvectorlayercache.h"

#include "qgsvectorlayereditbuffer.h"

QgsVectorLayerCache::QgsVectorLayerCache(QgsVectorLayer* layer)
  : L(layer)
{
}

QgsVectorLayerCache::~QgsVectorLayerCache()
{
  // Destroy any cached geometries and clear the references to them
  deleteCachedGeometries();
}

bool QgsVectorLayerCache::geometry(QgsFeatureId fid, QgsGeometry& geometry)
{
  // no need to check changed geometries because all changed geometries are also cached

  // first time this geometry has changed since last commit
  if ( !mCachedGeometries.contains( fid ) )
    return false;

  geometry = mCachedGeometries[fid];
  return true;
}

void QgsVectorLayerCache::cacheGeometry( QgsFeatureId fid, const QgsGeometry& geom )
{
  mCachedGeometries[fid] = geom;
}


void QgsVectorLayerCache::deleteCachedGeometries()
{
  // Destroy any cached geometries
  mCachedGeometries.clear();
  mCachedGeometriesRect = QgsRectangle();
}
