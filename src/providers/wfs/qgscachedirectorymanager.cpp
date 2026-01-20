/***************************************************************************
    qgscachedirectorymanager.cpp
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscachedirectorymanager.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include "moc_qgscachedirectorymanager.cpp"

// 1 minute
#define KEEP_ALIVE_DELAY ( 60 * 1000 )

#include <QFile>
#include <QDir>
#include <QTimer>
#include <QDateTime>

#if not defined( Q_OS_ANDROID )
#include <QSharedMemory>
#include <memory>
#endif

// -------------------------

std::map<QString, std::unique_ptr<QgsCacheDirectoryManager>> QgsCacheDirectoryManager::sMap;

QgsCacheDirectoryManager &QgsCacheDirectoryManager::singleton( const QString &providerName )
{
  static const QMutex sMapMutex;
  const auto iter = sMap.find( providerName );
  if ( iter != sMap.end() )
    return *( iter->second.get() );
  sMap[providerName] = std::unique_ptr<QgsCacheDirectoryManager>( new QgsCacheDirectoryManager( providerName ) );
  return *( sMap[providerName].get() );
}

QgsCacheDirectoryManager::QgsCacheDirectoryManager( const QString &providerName )
  : mProviderName( providerName )
{
  init();
}


QString QgsCacheDirectoryManager::getBaseCacheDirectory( bool createIfNotExisting )
{
  const QgsSettings settings;
  QString cacheDirectory = settings.value( u"cache/directory"_s ).toString();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QgsApplication::qgisSettingsDirPath() + "cache";
  const QString subDir = mProviderName + u"provider"_s;
  if ( createIfNotExisting )
  {
    const QMutexLocker locker( &mMutex );
    if ( !QDir( cacheDirectory ).exists( subDir ) )
    {
      QgsDebugMsgLevel( u"Creating main cache dir %1/%2"_s.arg( cacheDirectory ).arg( subDir ), 2 );
      QDir( cacheDirectory ).mkpath( subDir );
    }
  }
  return QDir( cacheDirectory ).filePath( subDir );
}

QString QgsCacheDirectoryManager::getCacheDirectory( bool createIfNotExisting )
{
  const QString baseDirectory( getBaseCacheDirectory( createIfNotExisting ) );
  const QString processPath( u"pid_%1"_s.arg( QCoreApplication::applicationPid() ) );
  if ( createIfNotExisting )
  {
    const QMutexLocker locker( &mMutex );
    if ( !QDir( baseDirectory ).exists( processPath ) )
    {
      QgsDebugMsgLevel( u"Creating our cache dir %1/%2"_s.arg( baseDirectory, processPath ), 2 );
      QDir( baseDirectory ).mkpath( processPath );
    }
#if not defined( Q_OS_ANDROID )
    if ( mCounter == 0 && mKeepAliveWorks )
    {
      mThread = new QgsCacheDirectoryManagerKeepAlive( createAndAttachSHM() );
      mThread->start();
    }
#endif
    mCounter++;
  }
  return QDir( baseDirectory ).filePath( processPath );
}

QString QgsCacheDirectoryManager::acquireCacheDirectory()
{
  return getCacheDirectory( true );
}

void QgsCacheDirectoryManager::releaseCacheDirectory()
{
  const QMutexLocker locker( &mMutex );
  mCounter--;
  if ( mCounter == 0 )
  {
    if ( mThread )
    {
      mThread->exit();
      mThread->wait();
      delete mThread;
      mThread = nullptr;
    }

    // Destroys our cache directory, and the main cache directory if it is empty
    const QString tmpDirname( getCacheDirectory( false ) );
    if ( QDir( tmpDirname ).exists() )
    {
      QgsDebugMsgLevel( u"Removing our cache dir %1"_s.arg( tmpDirname ), 2 );
      removeDir( tmpDirname );

      const QString baseDirname( getBaseCacheDirectory( false ) );
      const QDir baseDir( baseDirname );
      const QFileInfoList fileList( baseDir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
      if ( fileList.size() == 0 )
      {
        QgsDebugMsgLevel( u"Removing main cache dir %1"_s.arg( baseDirname ), 2 );
        removeDir( baseDirname );
      }
      else
      {
        QgsDebugMsgLevel( u"%1 entries remaining in %2"_s.arg( fileList.size() ).arg( baseDirname ), 2 );
      }
    }
  }
}

bool QgsCacheDirectoryManager::removeDir( const QString &dirName )
{
  const QDir dir( dirName );
  const QFileInfoList fileList( dir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
  const auto constFileList = fileList;
  for ( const QFileInfo &info : constFileList )
  {
    bool result;
    if ( info.isDir() )
    {
      result = removeDir( info.absoluteFilePath() );
    }
    else
    {
      result = QFile::remove( info.absoluteFilePath() );
    }

    if ( !result )
      break;
  }
  return dir.rmdir( dirName );
}

#if not defined( Q_OS_ANDROID )
std::unique_ptr<QSharedMemory> QgsCacheDirectoryManager::createAndAttachSHM()
{
  std::unique_ptr<QSharedMemory> sharedMemory;
  // For debug purpose. To test in the case where shared memory mechanism doesn't work
  if ( !getenv( "QGIS_USE_SHARED_MEMORY_KEEP_ALIVE" ) )
  {
    sharedMemory = std::make_unique<QSharedMemory>( u"qgis_%1_pid_%2"_s.arg( mProviderName ).arg( QCoreApplication::applicationPid() ) );
    if ( sharedMemory->create( sizeof( qint64 ) ) && sharedMemory->lock() && sharedMemory->unlock() )
    {
      return sharedMemory;
    }
    else
    {
      // Would happen on Unix in the quite unlikely situation where a past process
      // with the same PID as ours would have been killed before it destroyed
      // its shared memory segment. So we will recycle it.
      if ( sharedMemory->error() == QSharedMemory::AlreadyExists && sharedMemory->attach() && sharedMemory->size() == static_cast<int>( sizeof( qint64 ) ) )
      {
        return sharedMemory;
      }
    }
  }
  return nullptr;
}
#endif

void QgsCacheDirectoryManager::init()
{
#if not defined( Q_OS_ANDROID )
  auto sharedMemory = createAndAttachSHM();
  mKeepAliveWorks = sharedMemory.get() != nullptr;
  sharedMemory.reset();
#else
  mKeepAliveWorks = false;
#endif

  if ( mKeepAliveWorks )
  {
    QgsDebugMsgLevel( u"Keep-alive mechanism works"_s, 4 );
  }
  else
  {
    QgsDebugMsgLevel( u"Keep-alive mechanism does not work"_s, 4 );
  }

  // Remove temporary directories of qgis instances that haven't demonstrated
  // a sign of life since 2 * KEEP_ALIVE_DELAY
  const QDir dir( getBaseCacheDirectory( false ) );
  if ( dir.exists() )
  {
    const qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    const QFileInfoList fileList( dir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
    const auto constFileList = fileList;
    for ( const QFileInfo &info : constFileList )
    {
      if ( info.isDir() && info.fileName().startsWith( "pid_"_L1 ) )
      {
        const QString pidStr( info.fileName().mid( 4 ) );
        const qint64 pid = pidStr.toLongLong();
        bool canDelete = false;
        if ( pid == QCoreApplication::applicationPid() )
        {
          canDelete = true;
        }
#if not defined( Q_OS_ANDROID )
        else if ( mKeepAliveWorks )
        {
          canDelete = true;
          QSharedMemory otherSharedMemory( u"qgis_%1_pid_%2"_s.arg( mProviderName ).arg( pid ) );
          if ( otherSharedMemory.attach() )
          {
            if ( otherSharedMemory.size() == sizeof( qint64 ) )
            {
              if ( otherSharedMemory.lock() )
              {
                qint64 otherTimestamp;
                memcpy( &otherTimestamp, otherSharedMemory.data(), sizeof( qint64 ) );
                otherSharedMemory.unlock();
                if ( currentTimestamp > otherTimestamp && otherTimestamp > 0 && currentTimestamp - otherTimestamp < 2 * KEEP_ALIVE_DELAY )
                {
                  QgsDebugMsgLevel( u"Cache dir %1 kept since process seems to be still alive"_s.arg( info.absoluteFilePath() ), 4 );
                  canDelete = false;
                }
                else
                {
                  QgsDebugMsgLevel( u"Cache dir %1 to be destroyed since process seems to be no longer alive"_s.arg( info.absoluteFilePath() ), 4 );
                }

                otherSharedMemory.unlock();
              }
            }
            otherSharedMemory.detach();
          }
          else
          {
            QgsDebugError( u"Cannot attach to shared memory segment of process %1. It must be ghost"_s.arg( pid ) );
          }
        }
#endif
        else
        {
          // Fallback to a file timestamp based method, if for some reason,
          // the shared memory stuff doesn't seem to work
          const qint64 fileTimestamp = info.lastModified().toMSecsSinceEpoch();
          if ( currentTimestamp > fileTimestamp && currentTimestamp - fileTimestamp < 24 * 3600 * 1000 )
          {
            QgsDebugMsgLevel( u"Cache dir %1 kept since last modified in the past 24 hours"_s.arg( info.absoluteFilePath() ), 4 );
            canDelete = false;
          }
          else
          {
            QgsDebugMsgLevel( u"Cache dir %1 to be destroyed since not modified in the past 24 hours"_s.arg( info.absoluteFilePath() ), 4 );
            canDelete = true;
          }
        }
        if ( canDelete )
        {
          QgsDebugMsgLevel( u"Removing cache dir %1"_s.arg( info.absoluteFilePath() ), 4 );
          removeDir( info.absoluteFilePath() );
        }
      }
    }
  }
}

// -------------------------

#if not defined( Q_OS_ANDROID )
// We use a keep alive mechanism where every KEEP_ALIVE_DELAY ms we update
// a shared memory segment with the current timestamp. This way, other QGIS
// processes can check if the temporary directories of other process correspond
// to alive or ghost processes.
QgsCacheDirectoryManagerKeepAlive::QgsCacheDirectoryManagerKeepAlive( std::unique_ptr<QSharedMemory> &&sharedMemory )
  : mSharedMemory( std::move( sharedMemory ) )
{
  updateTimestamp();
}

QgsCacheDirectoryManagerKeepAlive::~QgsCacheDirectoryManagerKeepAlive()
{
}

void QgsCacheDirectoryManagerKeepAlive::updateTimestamp()
{
  qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
  if ( mSharedMemory->lock() )
  {
    QgsDebugMsgLevel( u"Updating keep-alive"_s, 4 );
    memcpy( mSharedMemory->data(), &timestamp, sizeof( timestamp ) );
    mSharedMemory->unlock();
  }
}

void QgsCacheDirectoryManagerKeepAlive::run()
{
  QTimer timer;
  timer.setInterval( KEEP_ALIVE_DELAY );
  timer.start();
  connect( &timer, &QTimer::timeout, this, &QgsCacheDirectoryManagerKeepAlive::updateTimestamp );
  QThread::exec();
}
#endif
