/***************************************************************************
                              qgscapabilitiescache.h
                              ----------------------
  begin                : May 11th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscapabilitiescache.h"
#include "moc_qgscapabilitiescache.cpp"

#include <QCoreApplication>
#include <QFileInfo>

#if defined( Q_OS_LINUX )
#include <sys/vfs.h>
#endif

#include "qgslogger.h"
#include "qgsserversettings.h"
#include "qgsmessagelog.h"


QgsCapabilitiesCache::QgsCapabilitiesCache( int size )
  : mCacheSize( size )
{
  QObject::connect( &mFileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &QgsCapabilitiesCache::removeChangedEntry );

#if defined( Q_OS_LINUX )
  QObject::connect( &mTimer, &QTimer::timeout, this, &QgsCapabilitiesCache::removeOutdatedEntries );
#endif
}

const QDomDocument *QgsCapabilitiesCache::searchCapabilitiesDocument( const QString &configFilePath, const QString &key )
{
  QCoreApplication::processEvents(); //get updates from file system watcher

  if ( mCachedCapabilities.contains( configFilePath ) && mCachedCapabilities[configFilePath].contains( key ) )
  {
    return &mCachedCapabilities[configFilePath][key];
  }
  else
  {
    return nullptr;
  }
}

void QgsCapabilitiesCache::insertCapabilitiesDocument( const QString &configFilePath, const QString &key, const QDomDocument *doc )
{
  if ( mCachedCapabilities.size() > mCacheSize )
  {
    //remove another cache entry to avoid memory problems
    const QHash<QString, QHash<QString, QDomDocument>>::iterator capIt = mCachedCapabilities.begin();
    mFileSystemWatcher.removePath( capIt.key() );
    mCachedCapabilities.erase( capIt );

    QgsMessageLog::logMessage( QStringLiteral( "Removed cached WMS capabilities document because all %1 cache slots were taken" ).arg( mCacheSize ), QStringLiteral( "Server" ) );
  }

  if ( !mCachedCapabilities.contains( configFilePath ) )
  {
    mFileSystemWatcher.addPath( configFilePath );
    mCachedCapabilities.insert( configFilePath, QHash<QString, QDomDocument>() );
  }

  mCachedCapabilities[configFilePath].insert( key, doc->cloneNode().toDocument() );

#if defined( Q_OS_LINUX )
  struct statfs sStatFS;
  if ( statfs( configFilePath.toUtf8().constData(), &sStatFS ) == 0 && ( sStatFS.f_type == 0x6969 /* NFS */ || sStatFS.f_type == 0x517b /* SMB */ || sStatFS.f_type == 0xff534d42ul /* CIFS */ || sStatFS.f_type == 0xfe534d42ul /* CIFS */ ) )
  {
    const QFileInfo fi( configFilePath );
    mCachedCapabilitiesTimestamps[configFilePath] = fi.lastModified();
    mTimer.start( 1000 );
  }
#endif
}

void QgsCapabilitiesCache::removeCapabilitiesDocument( const QString &path )
{
  mCachedCapabilities.remove( path );
  mCachedCapabilitiesTimestamps.remove( path );
  mFileSystemWatcher.removePath( path );
}

void QgsCapabilitiesCache::removeChangedEntry( const QString &path )
{
  QgsDebugMsgLevel( QStringLiteral( "Remove capabilities cache entry because file changed" ), 2 );
  removeCapabilitiesDocument( path );
}

void QgsCapabilitiesCache::removeOutdatedEntries()
{
  QgsDebugMsgLevel( QStringLiteral( "Checking for outdated entries" ), 2 );
  for ( auto it = mCachedCapabilitiesTimestamps.constBegin(); it != mCachedCapabilitiesTimestamps.constEnd(); it++ )
  {
    const QString configFilePath = it.key();
    const QFileInfo fi( configFilePath );
    if ( !fi.exists() || it.value() < fi.lastModified() )
      removeChangedEntry( configFilePath );
  }

  if ( !mCachedCapabilitiesTimestamps.isEmpty() )
  {
    mTimer.start( 1000 );
  }
}
