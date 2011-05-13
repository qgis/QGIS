/***************************************************************************
                              qgsconfigcache.cpp
                              ------------------
  begin                : July 24th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
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

#include "qgsconfigcache.h"
#include "qgsmapserverlogger.h"
#include "qgsmslayercache.h"
#include "qgsprojectparser.h"
#include "qgssldparser.h"
#include <QCoreApplication>

QgsConfigCache::QgsConfigCache()
{
  QObject::connect( &mFileSystemWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( removeChangedEntry( const QString& ) ) );
}

QgsConfigCache::~QgsConfigCache()
{
  QHash<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.begin();
  for ( ; configIt != mCachedConfigurations.end(); ++configIt )
  {
    delete configIt.value();
  }
}

QgsConfigParser* QgsConfigCache::searchConfiguration( const QString& filePath )
{
  QCoreApplication::processEvents(); //check for updates from file system watcher
  QgsConfigParser* p = 0;
  QHash<QString, QgsConfigParser*>::const_iterator configIt = mCachedConfigurations.find( filePath );
  if ( configIt == mCachedConfigurations.constEnd() )
  {
    QgsMSDebugMsg( "Create new configuration" );
    p = insertConfiguration( filePath );
  }
  else
  {
    QgsMSDebugMsg( "Return configuration from cache" );
    p = configIt.value();
  }

  if ( p )
  {
    //there could be more layers in a project than allowed by the cache per default
    QgsMSLayerCache::instance()->setProjectMaxLayers( p->numberOfLayers() );
  }
  return p;
}

QgsConfigParser* QgsConfigCache::insertConfiguration( const QString& filePath )
{
  if ( mCachedConfigurations.size() > 40 )
  {
    //remove a cache entry to avoid memory problems
    QHash<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.begin();
    if ( configIt != mCachedConfigurations.end() )
    {
      mFileSystemWatcher.removePath( configIt.key() );
      delete configIt.value();
      mCachedConfigurations.erase( configIt );
    }
  }

  //first open file
  QFile* configFile = new QFile( filePath );
  if ( !configFile->exists() || !configFile->open( QIODevice::ReadOnly ) )
  {
    QgsMSDebugMsg( "File unreadable: " + filePath );
    delete configFile;
    return 0;
  }

  //then create xml document
  QDomDocument* configDoc = new QDomDocument();
  QString errorMsg;
  int line, column;
  if ( !configDoc->setContent( configFile, true, &errorMsg, &line, &column ) )
  {
    QgsMSDebugMsg( QString( "Parse error %1 at row %2, column %3 in %4 " )
                   .arg( errorMsg ).arg( line ).arg( column ).arg( filePath ) );
    delete configFile;
    delete configDoc;
    return 0;
  }

  //is it an sld document or a qgis project file?
  QDomElement documentElem = configDoc->documentElement();
  QgsConfigParser* configParser = 0;
  if ( documentElem.tagName() == "StyledLayerDescriptor" )
  {
    configParser = new QgsSLDParser( configDoc );
  }
  else if ( documentElem.tagName() == "qgis" )
  {
    configParser = new QgsProjectParser( configDoc, filePath );
  }
  else
  {
    QgsMSDebugMsg( "SLD or qgis expected in " + filePath );
    delete configDoc;
    return 0;
  }

  mCachedConfigurations.insert( filePath, configParser );
  mFileSystemWatcher.addPath( filePath );
  delete configFile;
  return configParser;
}

void QgsConfigCache::removeChangedEntry( const QString& path )
{
  QgsMSDebugMsg( "Remove config cache entry because file changed" );
  QHash<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.find( path );
  if ( configIt != mCachedConfigurations.end() )
  {
    delete configIt.value();
    mCachedConfigurations.erase( configIt );
  }
  mFileSystemWatcher.removePath( path );
}
