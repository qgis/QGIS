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

QgsNetworkDiskCache::ExpirableNetworkDiskCache QgsNetworkDiskCache::smDiskCache;
QMutex QgsNetworkDiskCache::smDiskCacheMutex;

QgsNetworkDiskCache::QgsNetworkDiskCache( QObject *parent )
    : QNetworkDiskCache( parent )
{
}

QgsNetworkDiskCache::~QgsNetworkDiskCache()
{
}

QString QgsNetworkDiskCache::cacheDirectory() const
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.cacheDirectory();
}

void QgsNetworkDiskCache::setCacheDirectory( const QString &cacheDir )
{
  QMutexLocker lock( &smDiskCacheMutex );
  smDiskCache.setCacheDirectory( cacheDir );
}

qint64 QgsNetworkDiskCache::maximumCacheSize() const
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.maximumCacheSize();
}

void QgsNetworkDiskCache::setMaximumCacheSize( qint64 size )
{
  QMutexLocker lock( &smDiskCacheMutex );
  smDiskCache.setMaximumCacheSize( size );
}

qint64 QgsNetworkDiskCache::cacheSize() const
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.cacheSize();
}

QNetworkCacheMetaData QgsNetworkDiskCache::metaData( const QUrl &url )
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.metaData( url );
}

void QgsNetworkDiskCache::updateMetaData( const QNetworkCacheMetaData &metaData )
{
  QMutexLocker lock( &smDiskCacheMutex );
  smDiskCache.updateMetaData( metaData );
}

QIODevice *QgsNetworkDiskCache::data( const QUrl &url )
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.data( url );
}

bool QgsNetworkDiskCache::remove( const QUrl &url )
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.remove( url );
}

QIODevice *QgsNetworkDiskCache::prepare( const QNetworkCacheMetaData &metaData )
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.prepare( metaData );
}

void QgsNetworkDiskCache::insert( QIODevice *device )
{
  QMutexLocker lock( &smDiskCacheMutex );
  smDiskCache.insert( device );
}

QNetworkCacheMetaData QgsNetworkDiskCache::fileMetaData( const QString &fileName ) const
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.fileMetaData( fileName );
}

qint64 QgsNetworkDiskCache::expire()
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.runExpire();
}

void QgsNetworkDiskCache::clear()
{
  QMutexLocker lock( &smDiskCacheMutex );
  return smDiskCache.clear();
}
