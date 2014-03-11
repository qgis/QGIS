
#include "qgsmaprenderercache.h"


QgsMapRendererCache::QgsMapRendererCache()
{
  clear();
}

void QgsMapRendererCache::clear()
{
  QMutexLocker lock( &mMutex );
  mExtent.setMinimal();
  mScale = 0;
  mCachedImages.clear();
}

bool QgsMapRendererCache::init( QgsRectangle extent, double scale )
{
  QMutexLocker lock( &mMutex );

  // check whether the params are the same
  if ( extent == mExtent &&
       scale == mScale )
    return true;

  // set new params
  mExtent = extent;
  mScale = scale;

  // invalidate cache
  mCachedImages.clear();

  return false;
}

void QgsMapRendererCache::setCacheImage( QString layerId, const QImage& img )
{
  QMutexLocker lock( &mMutex );
  mCachedImages[layerId] = img;
}

QImage QgsMapRendererCache::cacheImage( QString layerId )
{
  QMutexLocker lock( &mMutex );
  return mCachedImages.value( layerId );
}

void QgsMapRendererCache::layerDataChanged()
{
  // TODO!
  qDebug( "nothing here yet" );
}


