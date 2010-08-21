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
#include "qgsmapserverlogger.h"

#define MAX_N_LAYERS 50

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

}

QgsMSLayerCache::~QgsMSLayerCache()
{
  QgsMSDebugMsg( "Destructor QgsMSLayerCache: removing all entries" )
  QMap<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it;
  for ( it = mEntries.begin(); it != mEntries.end(); ++it )
  {
    delete it->layerPointer;
  }
  delete mInstance;
}

void QgsMSLayerCache::insertLayer( const QString& url, const QString& layerName, QgsMapLayer* layer, const QList<QString>& tempFiles )
{
  QgsMSDebugMsg( "inserting layer" )
  if ( mEntries.size() > MAX_N_LAYERS ) //force cache layer examination after 10 inserted layers
  {
    updateEntries();
  }

  QPair<QString, QString> urlLayerPair = qMakePair( url, layerName );
  QMap<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it = mEntries.find( urlLayerPair );
  if ( it != mEntries.end() )
  {
    delete it->layerPointer;
  }

  QgsMSLayerCacheEntry newEntry;
  newEntry.layerPointer = layer;
  newEntry.url = url;
  newEntry.creationTime = time( NULL );
  newEntry.lastUsedTime = time( NULL );
  newEntry.temporaryFiles = tempFiles;

  mEntries.insert( qMakePair( url, layerName ), newEntry );
}

QgsMapLayer* QgsMSLayerCache::searchLayer( const QString& url, const QString& layerName )
{
  QPair<QString, QString> urlNamePair = qMakePair( url, layerName );
  QMap<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it = mEntries.find( urlNamePair );
  if ( it == mEntries.end() )
  {
    return 0;
  }
  else
  {
    it->lastUsedTime = time( NULL );
#ifdef DIAGRAMSERVER
    //delete any existing diagram overlays in vectorlayers
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( it->layerPointer );
    if ( vl )
    {
      vl->removeOverlay( "diagram" );
    }
#endif //DIAGRAMSERVER
    return it->layerPointer;
  }
}

void QgsMSLayerCache::updateEntries()
{
  QgsMSDebugMsg( "updateEntries" )
  int entriesToDelete = mEntries.size() - MAX_N_LAYERS;
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
  QgsMSDebugMsg( "removeLeastUsedEntry" )
  QMap<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator it = mEntries.begin();
  QMap<QPair<QString, QString>, QgsMSLayerCacheEntry>::iterator lowest_it = it;
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
  //todo: remove the temporary files of a layer
  QList<QString>::const_iterator it = entry.temporaryFiles.constBegin();
  for ( ; it != entry.temporaryFiles.constEnd(); ++it )
  {
    //remove the temporary file
    QFile removeFile( *it );
    if ( !removeFile.remove() )
    {
      QgsMSDebugMsg( "QgsMSLayerCache::freeEntryRessources: could not remove file: " + *it )
      QgsMSDebugMsg( removeFile.errorString() )
    }
  }
}


