/***************************************************************************
    qgswfsutils.cpp
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

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgswfsutils.h"
#include "qgsgeometry.h"
#include "qgssettings.h"

// 1 minute
#define KEEP_ALIVE_DELAY        (60 * 1000)

#include <QFile>
#include <QDir>
#include <QTimer>
#include <QSharedMemory>
#include <QDateTime>
#include <QCryptographicHash>

QMutex QgsWFSUtils::sMutex;
QThread *QgsWFSUtils::sThread = nullptr;
bool QgsWFSUtils::sKeepAliveWorks = false;
int QgsWFSUtils::sCounter = 0;

QString QgsWFSUtils::getBaseCacheDirectory( bool createIfNotExisting )
{
  QgsSettings settings;
  QString cacheDirectory = settings.value( QStringLiteral( "cache/directory" ) ).toString();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QgsApplication::qgisSettingsDirPath() + "cache";
  if ( createIfNotExisting )
  {
    QMutexLocker locker( &sMutex );
    if ( !QDir( cacheDirectory ).exists( QStringLiteral( "wfsprovider" ) ) )
    {
      QgsDebugMsg( QStringLiteral( "Creating main cache dir %1/wfsprovider" ).arg( cacheDirectory ) );
      QDir( cacheDirectory ).mkpath( QStringLiteral( "wfsprovider" ) );
    }
  }
  return QDir( cacheDirectory ).filePath( QStringLiteral( "wfsprovider" ) );
}

QString QgsWFSUtils::getCacheDirectory( bool createIfNotExisting )
{
  QString baseDirectory( getBaseCacheDirectory( createIfNotExisting ) );
  QString processPath( QStringLiteral( "pid_%1" ).arg( QCoreApplication::applicationPid() ) );
  if ( createIfNotExisting )
  {
    QMutexLocker locker( &sMutex );
    if ( !QDir( baseDirectory ).exists( processPath ) )
    {
      QgsDebugMsg( QStringLiteral( "Creating our cache dir %1/%2" ).arg( baseDirectory, processPath ) );
      QDir( baseDirectory ).mkpath( processPath );
    }
    if ( sCounter == 0 && sKeepAliveWorks )
    {
      sThread = new QgsWFSUtilsKeepAlive();
      sThread->start();
    }
    sCounter ++;
  }
  return QDir( baseDirectory ).filePath( processPath );
}

QString QgsWFSUtils::acquireCacheDirectory()
{
  return getCacheDirectory( true );
}

void QgsWFSUtils::releaseCacheDirectory()
{
  QMutexLocker locker( &sMutex );
  sCounter --;
  if ( sCounter == 0 )
  {
    if ( sThread )
    {
      sThread->exit();
      sThread->wait();
      delete sThread;
      sThread = nullptr;
    }

    // Destroys our cache directory, and the main cache directory if it is empty
    QString tmpDirname( getCacheDirectory( false ) );
    if ( QDir( tmpDirname ).exists() )
    {
      QgsDebugMsg( QStringLiteral( "Removing our cache dir %1" ).arg( tmpDirname ) );
      removeDir( tmpDirname );

      QString baseDirname( getBaseCacheDirectory( false ) );
      QDir baseDir( baseDirname );
      QFileInfoList fileList( baseDir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
      if ( fileList.size() == 0 )
      {
        QgsDebugMsg( QStringLiteral( "Removing main cache dir %1" ).arg( baseDirname ) );
        removeDir( baseDirname );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "%1 entries remaining in %2" ).arg( fileList.size() ).arg( baseDirname ) );
      }
    }
  }
}

bool QgsWFSUtils::removeDir( const QString &dirName )
{
  QDir dir( dirName );
  QFileInfoList fileList( dir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
  Q_FOREACH ( const QFileInfo &info, fileList )
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


// We use a keep alive mechanism where every KEEP_ALIVE_DELAY ms we update
// a shared memory segment with the current timestamp. This way, other QGIS
// processes can check if the temporary directories of other process correspond
// to alive or ghost processes.
QgsWFSUtilsKeepAlive::QgsWFSUtilsKeepAlive()
  : mSharedMemory( QgsWFSUtils::createAndAttachSHM() )
{
  updateTimestamp();
}

QgsWFSUtilsKeepAlive::~QgsWFSUtilsKeepAlive()
{
  delete mSharedMemory;
}

void QgsWFSUtilsKeepAlive::updateTimestamp()
{
  qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
  if ( mSharedMemory->lock() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Updating keep-alive" ), 4 );
    memcpy( mSharedMemory->data(), &timestamp, sizeof( timestamp ) );
    mSharedMemory->unlock();
  }
}

void QgsWFSUtilsKeepAlive::run()
{
  QTimer timer;
  timer.setInterval( KEEP_ALIVE_DELAY );
  timer.start();
  connect( &timer, &QTimer::timeout, this, &QgsWFSUtilsKeepAlive::updateTimestamp );
  QThread::exec();
}

QSharedMemory *QgsWFSUtils::createAndAttachSHM()
{
  QSharedMemory *sharedMemory = nullptr;
  // For debug purpose. To test in the case where shared memory mechanism doesn't work
  if ( !getenv( "QGIS_USE_SHARED_MEMORY_KEEP_ALIVE" ) )
  {
    sharedMemory = new QSharedMemory( QStringLiteral( "qgis_wfs_pid_%1" ).arg( QCoreApplication::applicationPid() ) );
    if ( sharedMemory->create( sizeof( qint64 ) ) && sharedMemory->lock() && sharedMemory->unlock() )
    {
      return sharedMemory;
    }
    else
    {
      // Would happen on Unix in the quite unlikely situation where a past process
      // with the same PID as ours would have been killed before it destroyed
      // its shared memory segment. So we will recycle it.
      if ( sharedMemory->error() == QSharedMemory::AlreadyExists &&
           sharedMemory->attach() && sharedMemory->size() == static_cast<int>( sizeof( qint64 ) ) )
      {
        return sharedMemory;
      }
    }
  }
  delete sharedMemory;
  return nullptr;
}

void QgsWFSUtils::init()
{
  QSharedMemory *sharedMemory = createAndAttachSHM();
  sKeepAliveWorks = static_cast< bool >( sharedMemory );
  delete sharedMemory;

  if ( sKeepAliveWorks )
  {
    QgsDebugMsgLevel( QStringLiteral( "Keep-alive mechanism works" ), 4 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Keep-alive mechanism does not work" ), 4 );
  }

  // Remove temporary directories of qgis instances that haven't demonstrated
  // a sign of life since 2 * KEEP_ALIVE_DELAY
  QDir dir( getBaseCacheDirectory( false ) );
  if ( dir.exists() )
  {
    const qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    QFileInfoList fileList( dir.entryInfoList( QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files ) );
    Q_FOREACH ( const QFileInfo &info, fileList )
    {
      if ( info.isDir() && info.fileName().startsWith( QLatin1String( "pid_" ) ) )
      {
        QString pidStr( info.fileName().mid( 4 ) );
        qint64 pid = pidStr.toLongLong();
        bool canDelete = false;
        if ( pid == QCoreApplication::applicationPid() )
        {
          canDelete = true;
        }
        else if ( sKeepAliveWorks )
        {
          canDelete = true;
          QSharedMemory otherSharedMemory( QStringLiteral( "qgis_wfs_pid_%1" ).arg( pid ) );
          if ( otherSharedMemory.attach() )
          {
            if ( otherSharedMemory.size() == sizeof( qint64 ) )
            {
              if ( otherSharedMemory.lock() )
              {
                qint64 otherTimestamp;
                memcpy( &otherTimestamp, otherSharedMemory.data(), sizeof( qint64 ) );
                otherSharedMemory.unlock();
                if ( currentTimestamp > otherTimestamp && otherTimestamp > 0 &&
                     currentTimestamp - otherTimestamp < 2 * KEEP_ALIVE_DELAY )
                {
                  QgsDebugMsgLevel( QStringLiteral( "Cache dir %1 kept since process seems to be still alive" ).arg( info.absoluteFilePath() ), 4 );
                  canDelete = false;
                }
                else
                {
                  QgsDebugMsgLevel( QStringLiteral( "Cache dir %1 to be destroyed since process seems to be no longer alive" ).arg( info.absoluteFilePath() ), 4 );
                }

                otherSharedMemory.unlock();
              }
            }
            otherSharedMemory.detach();
          }
          else
          {
            QgsDebugMsg( QStringLiteral( "Cannot attach to shared memory segment of process %1. It must be ghost" ).arg( pid ) );
          }
        }
        else
        {
          // Fallback to a file timestamp based method, if for some reason,
          // the shared memory stuff doesn't seem to work
          const qint64 fileTimestamp = info.lastModified().toMSecsSinceEpoch();
          if ( currentTimestamp > fileTimestamp &&
               currentTimestamp - fileTimestamp < 24 * 3600 * 1000 )
          {
            QgsDebugMsgLevel( QStringLiteral( "Cache dir %1 kept since last modified in the past 24 hours" ).arg( info.absoluteFilePath() ), 4 );
            canDelete = false;
          }
          else
          {
            QgsDebugMsgLevel( QStringLiteral( "Cache dir %1 to be destroyed since not modified in the past 24 hours" ).arg( info.absoluteFilePath() ), 4 );
            canDelete = true;
          }
        }
        if ( canDelete )
        {
          QgsDebugMsgLevel( QStringLiteral( "Removing cache dir %1" ).arg( info.absoluteFilePath() ), 4 );
          removeDir( info.absoluteFilePath() );
        }
      }
    }
  }
}


QString QgsWFSUtils::removeNamespacePrefix( const QString &tname )
{
  QString name( tname );
  if ( name.contains( ':' ) )
  {
    QStringList splitList = name.split( ':' );
    if ( splitList.size() > 1 )
    {
      name = splitList.at( 1 );
    }
  }
  return name;
}

QString QgsWFSUtils::nameSpacePrefix( const QString &tname )
{
  QStringList splitList = tname.split( ':' );
  if ( splitList.size() < 2 )
  {
    return QString();
  }
  return splitList.at( 0 );
}


QString QgsWFSUtils::getMD5( const QgsFeature &f )
{
  const QgsAttributes attrs = f.attributes();
  QCryptographicHash hash( QCryptographicHash::Md5 );
  for ( int i = 0; i < attrs.size(); i++ )
  {
    const QVariant &v = attrs[i];
    hash.addData( QByteArray( ( const char * )&i, sizeof( i ) ) );
    if ( v.isNull() )
    {
      // nothing to do
    }
    else if ( v.type() == QVariant::DateTime )
    {
      qint64 val = v.toDateTime().toMSecsSinceEpoch();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else if ( v.type() == QVariant::Int )
    {
      int val = v.toInt();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else if ( v.type() == QVariant::LongLong )
    {
      qint64 val = v.toLongLong();
      hash.addData( QByteArray( ( const char * )&val, sizeof( val ) ) );
    }
    else  if ( v.type() == QVariant::String )
    {
      hash.addData( v.toByteArray() );
    }
  }

  const int attrCount = attrs.size();
  hash.addData( QByteArray( ( const char * )&attrCount, sizeof( attrCount ) ) );
  QgsGeometry geometry = f.geometry();
  if ( !geometry.isNull() )
  {
    hash.addData( geometry.asWkb() );
  }

  return hash.result().toHex();
}
