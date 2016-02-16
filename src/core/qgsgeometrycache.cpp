/***************************************************************************
    qgsgeometrycache.cpp
    ---------------------
    begin                : March 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeometrycache.h"

#include "qgsvectorlayereditbuffer.h"

QgsGeometryCache::QgsGeometryCache()
{
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
