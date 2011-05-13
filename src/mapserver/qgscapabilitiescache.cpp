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
#include "qgsmapserverlogger.h"
#include <QCoreApplication>

QgsCapabilitiesCache::QgsCapabilitiesCache()
{
  QObject::connect( &mFileSystemWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( removeChangedEntry( const QString& ) ) );
}

QgsCapabilitiesCache::~QgsCapabilitiesCache()
{
}

const QDomDocument* QgsCapabilitiesCache::searchCapabilitiesDocument( const QString& configFilePath ) const
{
  QCoreApplication::processEvents(); //get updates from file system watcher
  QHash< QString, QDomDocument >::const_iterator it = mCachedCapabilities.find( configFilePath );
  if( it == mCachedCapabilities.constEnd() )
  {
    return 0;
  }
  else
  {
    return &(it.value());
  }
}

void QgsCapabilitiesCache::insertCapabilitiesDocument( const QString& configFilePath, const QDomDocument* doc )
{
  if( mCachedCapabilities.size() > 40 )
  {
    //remove another cache entry to avoid memory problems
    QHash<QString, QDomDocument>::iterator capIt = mCachedCapabilities.begin();
    mFileSystemWatcher.removePath( capIt.key() );
    mCachedCapabilities.erase( capIt );
  }
  mCachedCapabilities.insert( configFilePath, doc->cloneNode().toDocument() );
  mFileSystemWatcher.addPath( configFilePath );
}

void QgsCapabilitiesCache::removeChangedEntry( const QString& path )
{
  QgsMSDebugMsg( "Remove capabilities cache entry because file changed" );
  QHash< QString, QDomDocument >::iterator it = mCachedCapabilities.find( path );
  if( it != mCachedCapabilities.end() )
  {
    mCachedCapabilities.erase( it );
  }
  mFileSystemWatcher.removePath( path );
}
