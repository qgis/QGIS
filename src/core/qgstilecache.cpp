/***************************************************************************
  qgstilecache.h
  --------------------------------------
  Date                 : September 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstilecache.h"

#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"

#include <QAbstractNetworkCache>
#include <QImage>
#include <QUrl>

// Default cache size is 256 tiles (considering tiles 256x256)
// TODO: make this configurable!
QCache<QUrl, QImage> QgsTileCache::sTileCache( 256 );
QMutex QgsTileCache::sTileCacheMutex;


void QgsTileCache::insertTile( const QUrl &url, const QImage &image )
{
  const QMutexLocker locker( &sTileCacheMutex );
  // The cache default size is 256 tiles, but since this is also used for caching tiles
  // from cascading WMS servers which can become quite large, we use the number of pixels
  // in the image as multiple of a "standard" tile of 256x256 to calculate a cost for the cache.
  // This way, the cache will be able to store more smaller tiles and less larger tiles
  // using approximately the same amount of memory.
  sTileCache.insert( url, new QImage( image ), std::max( static_cast<qsizetype>( 1 ), static_cast<qsizetype>( image.width() * image.height() / ( 256 * 256 ) ) ) );
}

bool QgsTileCache::tile( const QUrl &url, QImage &image )
{
  QNetworkRequest req( url );
  //Preprocessing might alter the url, so we need to make sure we store/retrieve the url after preprocessing
  QgsNetworkAccessManager::instance()->preprocessRequest( &req );
  const QUrl adjUrl = req.url();

  const QMutexLocker locker( &sTileCacheMutex );
  bool success = false;
  if ( QImage *i = sTileCache.object( adjUrl ) )
  {
    image = *i;
    success = true;
  }
  else if ( QgsNetworkAccessManager::instance()->cache()->metaData( adjUrl ).isValid() )
  {
    if ( QIODevice *data = QgsNetworkAccessManager::instance()->cache()->data( adjUrl ) )
    {
      const QByteArray imageData = data->readAll();
      delete data;

      image = QImage::fromData( imageData );

      // cache it as well (mutex is already locked)
      // Check for null because it could be a redirect (see: https://github.com/qgis/QGIS/issues/24336 )
      if ( !image.isNull() )
      {
        sTileCache.insert( adjUrl, new QImage( image ) );
        success = true;
      }
    }
  }
  return success;
}

int QgsTileCache::totalCost()
{
  const QMutexLocker locker( &sTileCacheMutex );
  return sTileCache.totalCost();
}

int QgsTileCache::maxCost()
{
  const QMutexLocker locker( &sTileCacheMutex );
  return sTileCache.maxCost();
}
