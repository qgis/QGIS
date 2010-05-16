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

#include <QMap>
#include <QObject>

class QString;
class QStringList;

class QgsMapLayer;

/** \ingroup core
* This class tracks map layers that are currently loaded and provides
* a means to fetch a pointer to a map layer and delete it.
*/
class CORE_EXPORT QgsMapLayerRegistry : public QObject
{
    Q_OBJECT

  public:

//! Returns the instance pointer, creating the object on the first call
    static QgsMapLayerRegistry * instance();
    /*! Return the number of registered layers.
     *
     * */
    int count();

    ~QgsMapLayerRegistry();

//! Retrieve a pointer to a loaded plugin by id
    QgsMapLayer * mapLayer( QString theLayerId );

//! Retrieve the mapLayers collection (mainly intended for use by projectio)
    QMap<QString, QgsMapLayer*> & mapLayers();

    /** Add a layer to the map of loaded layers
       @returns NULL if unable to add layer, otherwise pointer to newly added layer
       @note

       As a side-effect QgsProject is made dirty.

       Emits signal that layer has been added only if theEmitSignal is true (by default).
       Not emitting signal is useful when you want to use registry also for layers
       which won't be used in main map canvas but will be used in a special one
    */
    QgsMapLayer *addMapLayer( QgsMapLayer * theMapLayer, bool theEmitSignal = true );

    /** Remove a layer from qgis
       @note
       As a side-effect QgsProject is made dirty.
       Any canvases using that layer will need to remove it
       theEmitSignal - see addMapLayer()
       The layer being removed is deleted as well as the registry
       table entry.
    */
    void removeMapLayer( QString theLayerId, bool theEmitSignal = true );

    /** Remove all registered layers
       @note raises removedAll()
       As a side-effect QgsProject is made dirty.
       @note The layers are deleted as the registry is cleared!
    */
    void removeAllMapLayers();

    /* Clears all layer caches, resetting them to zero and
     * freeing up any memory they may have been using. Layer
     * caches are used to speed up rendering in certain situations
     * see ticket #1974 for more details.
     * @note this method was added in QGIS 1.4
     */
    void clearAllLayerCaches();
  signals:

    /** emitted when a layer is removed from the registry
       connected to main map canvas and overview map canvas remove()
    */
    void layerWillBeRemoved( QString theLayerId );

    /** emitted when a layer is added to the registry
       connected to main map canvas and overview map canvas addLayer()
    */
    void layerWasAdded( QgsMapLayer * theMapLayer );

    /** emitted when ALL layers are removed at once
       This could have been implemented by iteratively signalling
       layerWillBeRemoved() for each layer as it is removed.  However, this
       generally causes a cascade of effects that are unnecessary if we're
       ultimately removing all layers.  E.g., removing the legend item
       corresponding to the layer.  Why bother doing that when you're just going
       to clear everything anyway?
     */
    void removedAll();

  protected:

//! protected constructor
    QgsMapLayerRegistry( QObject * parent = 0 );

  private:

    static QgsMapLayerRegistry* mInstance;

    QMap<QString, QgsMapLayer*> mMapLayers;

    /** debugging member
        invoked when a connect() is made to this object
    */
    void connectNotify( const char * signal );


}; // class QgsMapLayerRegistry

#endif //QgsMapLayerRegistry_H

