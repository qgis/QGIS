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
/* $Id$ */

#include <iostream>

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



QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer * theMapLayer, bool theEmitSignal )
{
  if ( !theMapLayer || !theMapLayer->isValid() )
  {
    QgsDebugMsg( "cannot add invalid layers" );
    return 0;
  }

  //check the layer is not already registered!
  QMap<QString, QgsMapLayer*>::iterator myIterator = mMapLayers.find( theMapLayer->getLayerID() );
  //if myIterator returns mMapLayers.end() then it does not exist in registry and its safe to add it
  if ( myIterator == mMapLayers.end() )
  {
    mMapLayers[theMapLayer->getLayerID()] = theMapLayer;

    if ( theEmitSignal )
      emit layerWasAdded( theMapLayer );

    return mMapLayers[theMapLayer->getLayerID()];
  }
  else
  {
    return 0;
  }
} //  QgsMapLayerRegistry::addMapLayer



void QgsMapLayerRegistry::removeMapLayer( QString theLayerId, bool theEmitSignal )
{
  if ( theEmitSignal )
    emit layerWillBeRemoved( theLayerId );
  delete mMapLayers[theLayerId];
  mMapLayers.remove( theLayerId );
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  // moved before physically removing the layers
  emit removedAll();

  // now let all canvas observers know to clear themselves,
  // and then consequently any of their map legends
  while ( mMapLayers.size() > 0 )
  {
    QString id = mMapLayers.begin().key();
    emit layerWillBeRemoved( id );
    delete mMapLayers[ id ]; // delete the map layer
    mMapLayers.remove( id );
  }

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

QMap<QString, QgsMapLayer*> & QgsMapLayerRegistry::mapLayers()
{
  return mMapLayers;
}



void QgsMapLayerRegistry::connectNotify( const char * signal )
{
  //QgsDebugMsg("QgsMapLayerRegistry connected to " + QString(signal));
} //  QgsMapLayerRegistry::connectNotify
