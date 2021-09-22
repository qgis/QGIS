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

#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include <QAbstractNetworkCache>
#include <QImage>
#include <QUrl>

QCache<QUrl, QImage> QgsTileCache::sTileCache( 256 );
QMutex QgsTileCache::sTileCacheMutex;


void QgsTileCache::insertTile( const QUrl &url, const QImage &image )
{
  const QMutexLocker locker( &sTileCacheMutex );
  sTileCache.insert( url, new QImage( image ) );
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
      if ( ! image.isNull( ) )
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
