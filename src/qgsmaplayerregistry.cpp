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
//
// Static calls to enforce singleton behaviour
//
QgsMapLayerRegistry *QgsMapLayerRegistry::mInstance = 0;
QgsMapLayerRegistry *QgsMapLayerRegistry::instance()
{
  if (mInstance == 0)
  {
    mInstance = new QgsMapLayerRegistry();
  }
  return mInstance;
}

//
// Main class begins now...
//

QgsMapLayerRegistry::QgsMapLayerRegistry(QObject *parent, const char *name) : QObject(parent,name) 
{
#ifdef QGISDEBUG
  std::cout << "---------------> qgs map layer registry created " << std::endl;
#endif
  // constructor does nothing
}


QgsMapLayer * QgsMapLayerRegistry::mapLayer(QString theLayerId)
{
  QgsMapLayer * myMapLayer = mMapLayers[theLayerId];
  if (myMapLayer)
  {
    return myMapLayer;
  }
  else
  {
    return 0;
  }
}

void QgsMapLayerRegistry::addMapLayer( QgsMapLayer * theMapLayer)
{
#ifdef QGISDEBUG
  std::cout << "qgsmaplayerregistry::addMaplayer - " << theMapLayer->name() << std::endl;
#endif
  mMapLayers[theMapLayer->getLayerID()] = theMapLayer;
  emit layerWasAdded(theMapLayer);
}

void QgsMapLayerRegistry::removeMapLayer(QString theLayerId)
{
#ifdef QGISDEBUG
  std::cout << "qgsmaplayerregistry::removemaplayer - emitting signal to notify all users of this layer to release it." << std::endl;
#endif
  emit layerWillBeRemoved(theLayerId); 
#ifdef QGISDEBUG
  std::cout << "qgsmaplayerregistry::removemaplayer - deleting map layer." << std::endl;
#endif
  delete mMapLayers[theLayerId]; 
#ifdef QGISDEBUG
  std::cout << "qgsmaplayerregistry::removemaplayer - unregistering map layer." << std::endl;
#endif
  mMapLayers.erase(theLayerId);
#ifdef QGISDEBUG
  std::cout << "qgsmaplayerregistry::removemaplayer - operation complete." << std::endl;
#endif
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  //emit layerWillBeRemoved(theLayerId); 
  //delete mMapLayers[theLayerId]; 
  //mMapLayers.erase(theLayerId);
}

std::map<QString,QgsMapLayer*> QgsMapLayerRegistry::mapLayers()
{
  return mMapLayers;
}
