/***************************************************************************
 *  QgsMapLayerRegistry.cpp  -  Singleton class for tracking mMapLayers.
 *                         -------------------
 * begin                : Sun June 02 2004
 * copyright            : (C) 2004 by Tim Sutton
 * email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"

//
// Static calls to enforce singleton behaviour
//
QgsMapLayerRegistry *QgsMapLayerRegistry::mInstance = 0;
QgsMapLayerRegistry *QgsMapLayerRegistry::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsMapLayerRegistry();
  }
  return mInstance;
}

//
// Main class begins now...
//

QgsMapLayerRegistry::QgsMapLayerRegistry( QObject *parent ) : QObject( parent )
{
  // constructor does nothing
}

QgsMapLayerRegistry::~QgsMapLayerRegistry()
{
  removeAllMapLayers();
}

// get the layer count (number of registered layers)
int QgsMapLayerRegistry::count()
{
  return mMapLayers.size();
}

QgsMapLayer * QgsMapLayerRegistry::mapLayer( QString theLayerId )
{
  return mMapLayers.value( theLayerId );
}

QList<QgsMapLayer *> QgsMapLayerRegistry::mapLayersByName( QString layerName )
{
  QList<QgsMapLayer *> myResultList;
  foreach ( QgsMapLayer* layer, mMapLayers )
  {
    if ( layer->name() == layerName )
    {
      myResultList << layer;
    }
  }
  return myResultList;
}

//introduced in 1.8
QList<QgsMapLayer *> QgsMapLayerRegistry::addMapLayers(
  QList<QgsMapLayer *> theMapLayers,
  bool addToLegend )
{
  QList<QgsMapLayer *> myResultList;
  for ( int i = 0; i < theMapLayers.size(); ++i )
  {
    QgsMapLayer * myLayer = theMapLayers.at( i );
    if ( !myLayer || !myLayer->isValid() )
    {
      QgsDebugMsg( "cannot add invalid layers" );
      continue;
    }
    //check the layer is not already registered!
    QMap<QString, QgsMapLayer*>::iterator myIterator =
      mMapLayers.find( myLayer->id() );
    //if myIterator returns mMapLayers.end() then it
    //does not exist in registry and its safe to add it
    if ( myIterator == mMapLayers.end() )
    {
      mMapLayers[myLayer->id()] = myLayer;
      myResultList << mMapLayers[myLayer->id()];
      emit layerWasAdded( myLayer );
    }
  }
  if ( myResultList.count() > 0 )
  {
    emit layersAdded( myResultList );

    if ( addToLegend )
      emit legendLayersAdded( myResultList );
  }
  return myResultList;
} // QgsMapLayerRegistry::addMapLayers

//this is just a thin wrapper for addMapLayers
QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer* theMapLayer,
                                  bool addToLegend )
{
  QList<QgsMapLayer *> addedLayers;
  addedLayers = addMapLayers( QList<QgsMapLayer*>() << theMapLayer, addToLegend );
  return addedLayers.isEmpty() ? 0 : addedLayers[0];
}


//introduced in 1.8
void QgsMapLayerRegistry::removeMapLayers( QStringList theLayerIds )
{
  emit layersWillBeRemoved( theLayerIds );

  foreach ( const QString &myId, theLayerIds )
  {
    emit layerWillBeRemoved( myId );
    delete mMapLayers[myId];
    mMapLayers.remove( myId );
  }
}

void QgsMapLayerRegistry::clearMapLayers()
{
  mMapLayers.clear();
} // QgsMapLayerRegistry::clearMapLayers()

void QgsMapLayerRegistry::removeMapLayer( const QString& theLayerId )
{
  removeMapLayers( QStringList( theLayerId ) );
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  emit removeAll();
  // now let all canvas observers know to clear themselves,
  // and then consequently any of their map legends
  removeMapLayers( mMapLayers.keys() );
  mMapLayers.clear();
} // QgsMapLayerRegistry::removeAllMapLayers()

//Added in QGIS 1.4
void QgsMapLayerRegistry::clearAllLayerCaches()
{
  QMap<QString, QgsMapLayer *>::iterator it;
  for ( it = mMapLayers.begin(); it != mMapLayers.end() ; ++it )
  {
    //the map layer will take care of deleting the QImage
    it.value()->setCacheImage( 0 );
  }
} // QgsMapLayerRegistry::clearAllLayerCaches()

void QgsMapLayerRegistry::reloadAllLayers()
{
  QMap<QString, QgsMapLayer *>::iterator it;
  for ( it = mMapLayers.begin(); it != mMapLayers.end() ; ++it )
  {
    QgsMapLayer* layer = it.value();
    if ( layer )
    {
      layer->reload();
    }
  }
}

const QMap<QString, QgsMapLayer*>& QgsMapLayerRegistry::mapLayers()
{
  return mMapLayers;
}



void QgsMapLayerRegistry::connectNotify( const char * signal )
{
  Q_UNUSED( signal );
  //QgsDebugMsg("QgsMapLayerRegistry connected to " + QString(signal));
} //  QgsMapLayerRegistry::connectNotify
