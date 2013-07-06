/***************************************************************************
                              qgsmslayercache.cpp
                              -------------------
  begin                : September 22, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmslayercache.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include <QFile>

//maximum number of layers in the cache
#define DEFAULT_MAX_N_LAYERS 100

QgsMSLayerCache* QgsMSLayerCache::mInstance = 0;

QgsMSLayerCache* QgsMSLayerCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsMSLayerCache();
  }
  return mInstance;
}

QgsMSLayerCache::QgsMSLayerCache()
{
  QObject::connect( &mFileSystemWatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( removeProjectFileLayers( const QString& ) ) );
}

QgsMSLayerCache::~QgsMSLayerCache()
{
  QgsDebugMsg( "removing all entries" );
  foreach ( QgsMSLayerCacheEntry entry, mEntries )
  {
    delete entry.layerPointer;
  }
  delete mInstance;
}

void QgsMSLayerCache::insertLayer( const QString& url, const QString& layerName, QgsMapLayer* layer, const QString& configFile, const QList<QString>& tempFiles )
{
  QgsDebugMsg( "inserting layer" );
  if ( mEntries.size() > std::max( DEFAULT_MAX_N_LAYERS, mProjectMaxLayers ) ) //force cache layer examination after 10 inserted layers
  {
    updateEntries();
  }

  QPair<QString, QString> urlLayerPair = qMakePair( url, layerName );
  QHash<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it = mEntries.find( urlLayerPair );
  if ( it != mEntries.end() )
  {
    delete it.value().layerPointer;
  }

  QgsMSLayerCacheEntry newEntry;
  newEntry.layerPointer = layer;
  newEntry.url = url;
  newEntry.creationTime = time( NULL );
  newEntry.lastUsedTime = time( NULL );
  newEntry.temporaryFiles = tempFiles;
  newEntry.configFile = configFile;

  mEntries.insert( urlLayerPair, newEntry );

  //update config file map
  if ( !configFile.isEmpty() )
  {
    QHash< QString, int >::iterator configIt = mConfigFiles.find( configFile );
    if ( configIt == mConfigFiles.end() )
    {
      mConfigFiles.insert( configFile, 1 );
      mFileSystemWatcher.addPath( configFile );
    }
    else
    {
      mConfigFiles[configFile] = configIt.value() + 1; //increment reference counter
    }
  }
}

QgsMapLayer* QgsMSLayerCache::searchLayer( const QString& url, const QString& layerName )
{
  QPair<QString, QString> urlNamePair = qMakePair( url, layerName );
  if ( !mEntries.contains( urlNamePair ) )
  {
    QgsDebugMsg( "Layer not found in cache" );
    return 0;
  }
  else
  {
    QgsMSLayerCacheEntry &entry = mEntries[ urlNamePair ];
    entry.lastUsedTime = time( NULL );
    QgsDebugMsg( "Layer found in cache" );
    return entry.layerPointer;
  }
}

void QgsMSLayerCache::removeProjectFileLayers( const QString& project )
{
  QList< QPair< QString, QString > > removeEntries;

  QHash<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator entryIt = mEntries.begin();
  for ( ; entryIt != mEntries.end(); ++entryIt )
  {
    if ( entryIt.value().configFile == project )
    {
      removeEntries.push_back( entryIt.key() );
      freeEntryRessources( entryIt.value() );
    }
  }

  QList< QPair< QString, QString > >::const_iterator removeIt = removeEntries.constBegin();
  for ( ; removeIt != removeEntries.constEnd(); ++removeIt )
  {
    mEntries.remove( *removeIt );
  }
}

void QgsMSLayerCache::updateEntries()
{
  QgsDebugMsg( "updateEntries" );
  int entriesToDelete = mEntries.size() - std::max( DEFAULT_MAX_N_LAYERS, mProjectMaxLayers );
  if ( entriesToDelete < 1 )
  {
    return;
  }

  for ( int i = 0; i < entriesToDelete; ++i )
  {
    removeLeastUsedEntry();
  }
}

void QgsMSLayerCache::removeLeastUsedEntry()
{

  if ( mEntries.size() < 1 )
  {
    return;
  }
  QgsDebugMsg( "removeLeastUsedEntry" );
  QHash<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it = mEntries.begin();
  QHash<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator lowest_it = it;
  time_t lowest_time = it->lastUsedTime;

  for ( ; it != mEntries.end(); ++it )
  {
    if ( it->lastUsedTime < lowest_time )
    {
      lowest_it = it;
      lowest_time = it->lastUsedTime;
    }
  }

  freeEntryRessources( *lowest_it );
  mEntries.erase( lowest_it );
}

void QgsMSLayerCache::freeEntryRessources( QgsMSLayerCacheEntry& entry )
{
  delete entry.layerPointer;

  //remove the temporary files of a layer
  foreach ( QString file, entry.temporaryFiles )
  {
    //remove the temporary file
    QFile removeFile( file );
    if ( !removeFile.remove() )
    {
      QgsDebugMsg( "could not remove file: " + file );
      QgsDebugMsg( removeFile.errorString() );
    }
  }

  //counter
  if ( !entry.configFile.isEmpty() )
  {
    int configFileCount = mConfigFiles[entry.configFile];
    if ( configFileCount < 2 )
    {
      mConfigFiles.remove( entry.configFile );
      mFileSystemWatcher.removePath( entry.configFile );
    }
    else
    {
      mConfigFiles[entry.configFile] = configFileCount - 1;
    }
  }
}
