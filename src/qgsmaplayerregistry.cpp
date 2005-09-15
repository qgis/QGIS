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
#include "qgsproject.h"


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
// get the layer count (number of registered layers)
const int QgsMapLayerRegistry::count()
{
  return mMapLayers.size();
}
 //! Get a vector layer from the registry - the the requested key does not exist or
 //does not correspond to a vector layer, null returned!
 QgsVectorLayer * QgsMapLayerRegistry::getVectorLayer(QString theLayerId)
{
  QgsVectorLayer * myVectorLayer = (QgsVectorLayer*) mMapLayers[theLayerId];
  if (myVectorLayer)
  {
    if (myVectorLayer->type() == QgsMapLayer::VECTOR)
    {
      return myVectorLayer;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
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



QgsMapLayer *
QgsMapLayerRegistry::addMapLayer( QgsMapLayer * theMapLayer )
{
#ifdef QGISDEBUG
  std::cout << "QgsMapLayerRegistry::addMaplayer - '" << theMapLayer->name().local8Bit() << "'."<< std::endl;
#endif
  //check the layer is not already registered!
  std::map<QString,QgsMapLayer*>::iterator myIterator = mMapLayers.find(theMapLayer->getLayerID());
  //if myIterator returns mMapLayers.end() then it does not exist in registry and its safe to add it
  if (myIterator == mMapLayers.end())
  {
    mMapLayers[theMapLayer->getLayerID()] = theMapLayer;
    emit layerWasAdded(theMapLayer);

    // notify the project we've made a change
    QgsProject::instance()->dirty(true);

    return mMapLayers[theMapLayer->getLayerID()];
  }
  else
  {

#ifdef QGISDEBUG
    std::cout << "addMaplayer - " << theMapLayer->name().local8Bit() << " already registered" << std::endl;
#endif
    return 0x0;
  }
} //  QgsMapLayerRegistry::addMapLayer



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
  // notify the project we've made a change
  QgsProject::instance()->dirty(true);
}

void QgsMapLayerRegistry::removeAllMapLayers()
{
  std::map<QString, QgsMapLayer *>::iterator myMapIterator = mMapLayers.begin();
  while ( myMapIterator != mMapLayers.end() )
  {
      delete myMapIterator->second; // delete the map layer

      mMapLayers.erase( myMapIterator );

      myMapIterator = mMapLayers.begin(); // since iterator invalidated due to
                                        // erase(), reset to new first element
  }

  emit removedAll();            // now let all canvas Observers know to clear
                                // themselves, and then consequently any of
                                // their map legends

  // notify the project we've made a change
  QgsProject::instance()->dirty(true);

} // QgsMapLayerRegistry::removeAllMapLayers()


std::map<QString,QgsMapLayer*> & QgsMapLayerRegistry::mapLayers()
{
  return mMapLayers;
}



void QgsMapLayerRegistry::connectNotify( const char * signal )
{
#ifdef QGISDEBUG
    std::cerr << "QgsMapLayerRegistry connected to " << signal << "\n";
#endif
} //  QgsMapLayerRegistry::connectNotify
