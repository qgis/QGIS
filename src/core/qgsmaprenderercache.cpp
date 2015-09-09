/***************************************************************************
  qgsmaprenderercache.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprenderercache.h"

#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"

QgsMapRendererCache::QgsMapRendererCache()
{
  clear();
}

void QgsMapRendererCache::clear()
{
  QMutexLocker lock( &mMutex );
  clearInternal();
}

void QgsMapRendererCache::clearInternal()
{
  mExtent.setMinimal();
  mScale = 0;

  // make sure we are disconnected from all layers
  Q_FOREACH ( const QString& layerId, mCachedImages.keys() )
  {
    QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    if ( layer )
    {
      disconnect( layer, SIGNAL( repaintRequested() ), this, SLOT( layerRequestedRepaint() ) );
    }
  }
  mCachedImages.clear();
}

bool QgsMapRendererCache::init( QgsRectangle extent, double scale )
{
  QMutexLocker lock( &mMutex );

  // check whether the params are the same
  if ( extent == mExtent &&
       scale == mScale )
    return true;

  clearInternal();

  // set new params
  mExtent = extent;
  mScale = scale;

  return false;
}

void QgsMapRendererCache::setCacheImage( QString layerId, const QImage& img )
{
  QMutexLocker lock( &mMutex );
  mCachedImages[layerId] = img;

  // connect to the layer to listen to layer's repaintRequested() signals
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( layer )
  {
    connect( layer, SIGNAL( repaintRequested() ), this, SLOT( layerRequestedRepaint() ) );
  }
}

QImage QgsMapRendererCache::cacheImage( QString layerId )
{
  QMutexLocker lock( &mMutex );
  return mCachedImages.value( layerId );
}

void QgsMapRendererCache::layerRequestedRepaint()
{
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( sender() );
  if ( layer )
    clearCacheImage( layer->id() );
}

void QgsMapRendererCache::clearCacheImage( QString layerId )
{
  QMutexLocker lock( &mMutex );

  mCachedImages.remove( layerId );

  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( layer )
  {
    disconnect( layer, SIGNAL( repaintRequested() ), this, SLOT( layerRequestedRepaint() ) );
  }
}
