/***************************************************************************
                          qgsmaplayerregistry.h    
           Singleton class for keeping track of loaded layers
                             -------------------
    begin                : Sun June 04 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
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
 
#ifndef QGSMAPLAYERREGISTRY_H
#define QGSMAPLAYERREGISTRY_H
#include <map>
#include <qobject.h>
#include "qgsmaplayer.h"
class QString;
class QStringList;
/**
* \class QgsMapLayerRegistry
* \brief This class tracks map layers that are currently loaded an provides
* a means to fetch a pointer to a map layer and delete it
*/
class QgsMapLayerRegistry : public QObject
{
Q_OBJECT;
public:
 //! Returns the instance pointer, creating the object on the first call
 static QgsMapLayerRegistry * instance();
 //! Retrieve a pointer to a loaded plugin by id
 QgsMapLayer * mapLayer(QString theLayerId);
 //! Retrieve the mapLayers collection (mainly intended for use by projectio)
 std::map<QString,QgsMapLayer*> mapLayers();
 //! Add a layer to the map of loaded layers
 void addMapLayer(QgsMapLayer * theMapLayer);
 //! Remove a layer from qgis - any canvases using that layer will need to remove it
 void removeMapLayer(QString theLayerId);
 //! Remove all registered layers 
 void removeAllMapLayers();
signals:
 void layerWillBeRemoved(QString theLayerId);
 void layerWasAdded(QgsMapLayer * theMapLayer);
protected:
//! protected constructor
 QgsMapLayerRegistry( QObject * parent = 0, const char * name = 0 );
private:
 static QgsMapLayerRegistry* mInstance;
 std::map<QString,QgsMapLayer*> mMapLayers;
};
#endif //QgsMapLayerRegistry_H

