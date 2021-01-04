/***************************************************************************
 qgsnetworkdiskcache.cpp  -  Thread-safe interface for QNetworkDiskCache
    -------------------
    begin                : 2016-03-05
    copyright            : (C) 2016 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnetworkdiskcache.h"

///@cond PRIVATE
ExpirableNetworkDiskCache QgsNetworkDiskCache::sDiskCache;
///@endcond
QMutex QgsNetworkDiskCache::sDiskCacheMutex;

QgsNetworkDiskCache::QgsNetworkDiskCache( QObject *parent )
  : QNetworkDiskCache( parent )
{
}

QString QgsNetworkDiskCache::cacheDirectory() const
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.cacheDirectory();
}

void QgsNetworkDiskCache::setCacheDirectory( const QString &cacheDir )
{
  QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.setCacheDirectory( cacheDir );
}

qint64 QgsNetworkDiskCache::maximumCacheSize() const
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.maximumCacheSize();
}

void QgsNetworkDiskCache::setMaximumCacheSize( qint64 size )
{
  QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.setMaximumCacheSize( size );
}

qint64 QgsNetworkDiskCache::cacheSize() const
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.cacheSize();
}

QNetworkCacheMetaData QgsNetworkDiskCache::metaData( const QUrl &url )
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.metaData( url );
}

void QgsNetworkDiskCache::updateMetaData( const QNetworkCacheMetaData &metaData )
{
  QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.updateMetaData( metaData );
}

QIODevice *QgsNetworkDiskCache::data( const QUrl &url )
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.data( url );
}

bool QgsNetworkDiskCache::remove( const QUrl &url )
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.remove( url );
}

QIODevice *QgsNetworkDiskCache::prepare( const QNetworkCacheMetaData &metaData )
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.prepare( metaData );
}

void QgsNetworkDiskCache::insert( QIODevice *device )
{
  QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.insert( device );
}

QNetworkCacheMetaData QgsNetworkDiskCache::fileMetaData( const QString &fileName ) const
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.fileMetaData( fileName );
}

qint64 QgsNetworkDiskCache::expire()
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.runExpire();
}

void QgsNetworkDiskCache::clear()
{
  QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.clear();
}
