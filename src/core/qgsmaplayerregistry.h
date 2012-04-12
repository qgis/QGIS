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

#ifndef QGSMAPLAYERREGISTRY_H
#define QGSMAPLAYERREGISTRY_H

#include <QMap>
#include <QObject>
#include <QStringList>
class QString;
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

    //! Retrieve a pointer to a loaded layer by id
    QgsMapLayer * mapLayer( QString theLayerId );

    //! Retrieve the mapLayers collection (mainly intended for use by projectio)
    QMap<QString, QgsMapLayer*> & mapLayers();

    /** Add a layer to the map of loaded layers
       @returns NULL if unable to add layer, otherwise pointer to newly added layer
       @note

       As a side-effect QgsProject is made dirty.

       Emits signal that layer has been added only if theEmitSignal is true (by default).

       Not emitting signal is useful when you want to use registry for layers
       on a different canvas and don't want them added to the main canvas automatically.

       @note This method is deprecated since QGIS 1.8, you should use addMapLayers rather.
    */
    Q_DECL_DEPRECATED QgsMapLayer *addMapLayer( QgsMapLayer * theMapLayer, bool theEmitSignal = true );

    /** Add a list of layers to the map of loaded layers
       @returns QList<QgsMapLayer *> - a list of the map layers that were added
       successfully. If a layer is invalid, or already exists in the registry,
       it will not be part of the returned QList.
       @note added in QGIS 1.8

       As a side-effect QgsProject is made dirty.

       If theEmitSignal is true (by default), a layersAdded( QList<QgsMapLayer *>)
       signal will be emitted indicating that a batch of layers were added.
       Not emitting signal is useful when you want to use registry for layers
       on a different canvas and don't want them added to the main canvas automatically.
    */
    QList<QgsMapLayer *> addMapLayers( QList<QgsMapLayer *> theMapLayers,
                                       bool theEmitSignal = true );


    /** Remove a layer from qgis
       @note As a side-effect QgsProject is made dirty. Any canvases using that layer will need to remove it

       If theEmitSignal is true (by default), a layerWillBeRemoved( QString theId )
       signal will be emitted indicating to any listeners that the layer is being removed.

       The layer being removed is deleted as well as the registry
       table entry.
       @note This method is deprecated since QGIS 1.8, you should use removeMapLayers rather.
    */
    Q_DECL_DEPRECATED void removeMapLayer( QString theLayerId, bool theEmitSignal = true );

    /** Remove a set of layers from qgis
       @note As a side-effect QgsProject is made dirty.
       Any canvases using the affected layers will need to remove them

       If theEmitSignal is true (by default), a layersRemoved( QStringList theLayerIds )
       signal will be emitted indicating to any listeners that the layers are being removed.

       The layer being removed is deleted as well as the registry
       table entry.
    */
    void removeMapLayers( QStringList theLayerIds, bool theEmitSignal = true );


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

    /**Reload all provider data caches (currently used for WFS and WMS providers)
      @note: this method was added in QGIS 1.6*/
    void reloadAllLayers();

  signals:

    /** Emitted when one or more layers are removed from the registry
       @note intended to replace layerWillBeRemoved in QGIS 1.8
    */
    void layersWillBeRemoved( QStringList theLayerIds );

    /** emitted when a layer is removed from the registry
       connected to main map canvas and overview map canvas remove()
       @note we should deprecate this at some stage
    */
    void layerWillBeRemoved( QString theLayerId );

    /** Emitted when one or more layers are added to the registry
       @note intended to replace layerWasAdded in QGIS 1.8
    */
    void layersAdded( QList<QgsMapLayer *> theMapLayers );
    /** emitted when a layer is added to the registry
       connected to main map canvas and overview map canvas addLayer()
       @note we should deprecate this at some stage
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

