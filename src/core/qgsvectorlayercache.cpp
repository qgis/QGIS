/***************************************************************************
    qgsvectorlayercache.cpp
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorlayercache.h"

#include "qgsvectorlayereditbuffer.h"

QgsVectorLayerCache::QgsVectorLayerCache( QgsVectorLayer* layer )
    : L( layer )
{
}

QgsVectorLayerCache::~QgsVectorLayerCache()
{
  // Destroy any cached geometries and clear the references to them
  deleteCachedGeometries();
}

bool QgsVectorLayerCache::geometry( QgsFeatureId fid, QgsGeometry& geometry )
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
