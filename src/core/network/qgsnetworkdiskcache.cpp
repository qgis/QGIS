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
#include "moc_qgsnetworkdiskcache.cpp"

#include <QStorageInfo>
#include <mutex>

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
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.cacheDirectory();
}

void QgsNetworkDiskCache::setCacheDirectory( const QString &cacheDir )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.setCacheDirectory( cacheDir );
}

qint64 QgsNetworkDiskCache::maximumCacheSize() const
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.maximumCacheSize();
}

void QgsNetworkDiskCache::setMaximumCacheSize( qint64 size )
{
  const QMutexLocker lock( &sDiskCacheMutex );

  if ( size == 0 )
  {
    // Calculate maximum cache size based on available free space
    size = smartCacheSize( sDiskCache.cacheDirectory() );
  }

  sDiskCache.setMaximumCacheSize( size );
}

qint64 QgsNetworkDiskCache::cacheSize() const
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.cacheSize();
}

QNetworkCacheMetaData QgsNetworkDiskCache::metaData( const QUrl &url )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.metaData( url );
}

void QgsNetworkDiskCache::updateMetaData( const QNetworkCacheMetaData &metaData )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.updateMetaData( metaData );
}

QIODevice *QgsNetworkDiskCache::data( const QUrl &url )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.data( url );
}

bool QgsNetworkDiskCache::remove( const QUrl &url )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.remove( url );
}

QIODevice *QgsNetworkDiskCache::prepare( const QNetworkCacheMetaData &metaData )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.prepare( metaData );
}

void QgsNetworkDiskCache::insert( QIODevice *device )
{
  const QMutexLocker lock( &sDiskCacheMutex );
  sDiskCache.insert( device );
}

QNetworkCacheMetaData QgsNetworkDiskCache::fileMetaData( const QString &fileName ) const
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.fileMetaData( fileName );
}

qint64 QgsNetworkDiskCache::expire()
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.runExpire();
}

void QgsNetworkDiskCache::clear()
{
  const QMutexLocker lock( &sDiskCacheMutex );
  return sDiskCache.clear();
}

qint64 QgsNetworkDiskCache::smartCacheSize( const QString &cacheDir )
{
  static qint64 cacheSize = 0;
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    std::function<qint64( const QString & )> dirSize;
    dirSize = [&dirSize]( const QString & dirPath ) -> qint64
    {
      qint64 size = 0;
      QDir dir( dirPath );

      const QStringList filePaths = dir.entryList( QDir::Files | QDir::System | QDir::Hidden );
      for ( const QString &filePath : filePaths )
      {
        QFileInfo fi( dir, filePath );
        size += fi.size();
      }

      const QStringList childDirPaths = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::NoSymLinks );
      for ( const QString &childDirPath : childDirPaths )
      {
        size += dirSize( dirPath + QDir::separator() + childDirPath );
      }

      return size;
    };

    qint64 bytesFree;
    QStorageInfo storageInfo( cacheDir );
    bytesFree = storageInfo.bytesFree() + dirSize( cacheDir );

    // NOLINTBEGIN(bugprone-narrowing-conversions)
    // Logic taken from Firefox's smart cache size handling
    qint64 available10MB = bytesFree / 1024 / ( 1024LL * 10 );
    qint64 cacheSize10MB = 0;
    if ( available10MB > 2500 )
    {
      // Cap the cache size to 1GB
      cacheSize10MB = 100;
    }
    else
    {
      if ( available10MB > 700 )
      {
        // Add 2.5% of the free space above 7GB
        cacheSize10MB += ( available10MB - 700 ) * 0.025;
        available10MB = 700;
      }
      if ( available10MB > 50 )
      {
        // Add 7.5% of free space between 500MB to 7GB
        cacheSize10MB += ( available10MB - 50 ) * 0.075;
        available10MB = 50;
      }

#if defined( Q_OS_ANDROID )
      // On Android, smaller/older devices may have very little storage

      // Add 16% of free space up to 500 MB
      cacheSize10MB += std::max( 2LL, static_cast<qint64>( available10MB * 0.16 ) );
#else
      // Add 30% of free space up to 500 MB
      cacheSize10MB += std::max( 5LL, static_cast<qint64>( available10MB * 0.30 ) );
#endif
    }
    cacheSize = cacheSize10MB * 10 * 1024 * 1024;
    // NOLINTEND(bugprone-narrowing-conversions)
  } );

  return cacheSize;
}
