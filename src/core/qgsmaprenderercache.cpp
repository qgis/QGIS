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

#include "qgsmaplayer.h"
#include "qgsmaplayerlistutils.h"
#include "qgsapplication.h"

#include <QImage>
#include <QPainter>
#include <algorithm>

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
  const auto constMConnectedLayers = mConnectedLayers;
  for ( const QgsWeakMapLayerPointer &layer : constMConnectedLayers )
  {
    if ( layer.data() )
    {
      disconnect( layer.data(), &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
      disconnect( layer.data(), &QgsMapLayer::willBeDeleted, this, &QgsMapRendererCache::layerRequestedRepaint );
    }
  }
  mCachedImages.clear();
  mConnectedLayers.clear();
}

void QgsMapRendererCache::dropUnusedConnections()
{
  QSet< QgsWeakMapLayerPointer > stillDepends = dependentLayers();
  QSet< QgsWeakMapLayerPointer > disconnects = mConnectedLayers.subtract( stillDepends );
  const auto constDisconnects = disconnects;
  for ( const QgsWeakMapLayerPointer &layer : constDisconnects )
  {
    if ( layer.data() )
    {
      disconnect( layer.data(), &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
      disconnect( layer.data(), &QgsMapLayer::willBeDeleted, this, &QgsMapRendererCache::layerRequestedRepaint );
    }
  }

  mConnectedLayers = stillDepends;
}

QSet<QgsWeakMapLayerPointer > QgsMapRendererCache::dependentLayers() const
{
  QSet< QgsWeakMapLayerPointer > result;
  QMap<QString, CacheParameters>::const_iterator it = mCachedImages.constBegin();
  for ( ; it != mCachedImages.constEnd(); ++it )
  {
    const auto dependentLayers { it.value().dependentLayers };
    for ( const QgsWeakMapLayerPointer &l : dependentLayers )
    {
      if ( l.data() )
        result << l;
    }
  }
  return result;
}

bool QgsMapRendererCache::init( const QgsRectangle &extent, double scale )
{
  QMutexLocker lock( &mMutex );

  // check whether the params are the same
  if ( extent == mExtent &&
       qgsDoubleNear( scale, mScale ) )
    return true;

  clearInternal();

  // set new params
  mExtent = extent;
  mScale = scale;
  mMtp = QgsMapToPixel::fromScale( scale, QgsUnitTypes::DistanceUnit::DistanceUnknownUnit );

  return false;
}

bool QgsMapRendererCache::updateParameters( const QgsRectangle &extent, const QgsMapToPixel &mtp )
{
  QMutexLocker lock( &mMutex );

  // check whether the params are the same
  if ( extent == mExtent &&
       mtp.transform() == mMtp.transform() )
    return true;

  // set new params

  mExtent = extent;
  mScale = 1.0;
  mMtp = mtp;

  return false;
}

void QgsMapRendererCache::setCacheImage( const QString &cacheKey, const QImage &image, const QList<QgsMapLayer *> &dependentLayers )
{
  QMutexLocker lock( &mMutex );

  CacheParameters params;
  params.cachedImage = image;
  params.cachedExtent = mExtent;
  params.cachedMtp = mMtp;

  // connect to the layer to listen to layer's repaintRequested() signals
  const auto constDependentLayers = dependentLayers;
  for ( QgsMapLayer *layer : constDependentLayers )
  {
    if ( layer )
    {
      params.dependentLayers << layer;
      if ( !mConnectedLayers.contains( QgsWeakMapLayerPointer( layer ) ) )
      {
        connect( layer, &QgsMapLayer::repaintRequested, this, &QgsMapRendererCache::layerRequestedRepaint );
        connect( layer, &QgsMapLayer::willBeDeleted, this, &QgsMapRendererCache::layerRequestedRepaint );
        mConnectedLayers << layer;
      }
    }
  }

  mCachedImages[cacheKey] = params;
}

bool QgsMapRendererCache::hasCacheImage( const QString &cacheKey ) const
{
  QMutexLocker lock( &mMutex );
  if ( mCachedImages.contains( cacheKey ) )
  {
    const CacheParameters params = mCachedImages[cacheKey];
    return ( params.cachedExtent == mExtent &&
             params.cachedMtp.transform() == mMtp.transform() );
  }
  else
  {
    return false;
  }
}

bool QgsMapRendererCache::hasAnyCacheImage( const QString &cacheKey ) const
{
  return mCachedImages.contains( cacheKey );
}

QImage QgsMapRendererCache::cacheImage( const QString &cacheKey ) const
{
  QMutexLocker lock( &mMutex );
  return mCachedImages.value( cacheKey ).cachedImage;
}

static QPointF _transform( const QgsMapToPixel &mtp, const QgsPointXY &point, double scale )
{
  qreal x = point.x(), y = point.y();
  mtp.transformInPlace( x, y );
  return QPointF( x, y ) * scale;
}

QImage QgsMapRendererCache::transformedCacheImage( const QString &cacheKey, const QgsMapToPixel &mtp ) const
{
  QMutexLocker lock( &mMutex );
  const CacheParameters params = mCachedImages.value( cacheKey );

  if ( params.cachedExtent == mExtent &&
       mtp.transform() == mMtp.transform() )
  {
    return params.cachedImage;
  }
  else
  {
    QgsRectangle intersection = mExtent.intersect( params.cachedExtent );
    if ( intersection.isNull() )
      return QImage();

    // Calculate target rect
    const QPointF ulT = _transform( mtp, QgsPointXY( intersection.xMinimum(), intersection.yMaximum() ), 1.0 );
    const QPointF lrT = _transform( mtp, QgsPointXY( intersection.xMaximum(), intersection.yMinimum() ), 1.0 );
    const QRectF targetRect( ulT.x(), ulT.y(), lrT.x() - ulT.x(), lrT.y() - ulT.y() );

    // Calculate source rect
    const QPointF ulS = _transform( params.cachedMtp, QgsPointXY( intersection.xMinimum(), intersection.yMaximum() ),  params.cachedImage.devicePixelRatio() );
    const QPointF lrS = _transform( params.cachedMtp, QgsPointXY( intersection.xMaximum(), intersection.yMinimum() ),  params.cachedImage.devicePixelRatio() );
    const QRectF sourceRect( ulS.x(), ulS.y(), lrS.x() - ulS.x(), lrS.y() - ulS.y() );

    // Draw image
    QImage ret( params.cachedImage.size(), params.cachedImage.format() );
    ret.setDevicePixelRatio( params.cachedImage.devicePixelRatio() );
    ret.setDotsPerMeterX( params.cachedImage.dotsPerMeterX() );
    ret.setDotsPerMeterY( params.cachedImage.dotsPerMeterY() );
    ret.fill( Qt::transparent );
    QPainter painter;
    painter.begin( &ret );
    painter.drawImage( targetRect, params.cachedImage, sourceRect );
    painter.end();
    return ret;
  }
}

QList< QgsMapLayer * > QgsMapRendererCache::dependentLayers( const QString &cacheKey ) const
{
  if ( mCachedImages.contains( cacheKey ) )
  {
    return _qgis_listQPointerToRaw( mCachedImages.value( cacheKey ).dependentLayers );
  }
  return QList< QgsMapLayer * >();
}


void QgsMapRendererCache::layerRequestedRepaint()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  invalidateCacheForLayer( layer );
}

void QgsMapRendererCache::invalidateCacheForLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  QMutexLocker lock( &mMutex );

  // check through all cached images to clear any which depend on this layer
  QMap<QString, CacheParameters>::iterator it = mCachedImages.begin();
  for ( ; it != mCachedImages.end(); )
  {
    if ( !it.value().dependentLayers.contains( layer ) )
    {
      ++it;
      continue;
    }

    it = mCachedImages.erase( it );
  }
  dropUnusedConnections();
}

void QgsMapRendererCache::clearCacheImage( const QString &cacheKey )
{
  QMutexLocker lock( &mMutex );

  mCachedImages.remove( cacheKey );
  dropUnusedConnections();
}

