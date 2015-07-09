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
#include <QSet>
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

    //! Return the number of registered layers.
    int count();

    ~QgsMapLayerRegistry();

    //! Retrieve a pointer to a loaded layer by id
    QgsMapLayer *mapLayer( QString theLayerId );

    //! Retrieve a pointer to a loaded layer by name
    QList<QgsMapLayer *> mapLayersByName( QString layerName );

    //! Retrieve the mapLayers collection (mainly intended for use by projection)
    const QMap<QString, QgsMapLayer*> & mapLayers();

    /**
     * @brief
     * Add a list of layers to the map of loaded layers
     *
     * The layersAdded() and layersWasAdded() signals will be emitted in any case.
     * The legendLayersAdded() signal only if addToLegend is true.
     *
     * @param theMapLayers  A list of layer which should be added to the registry
     * @param addToLegend   If true (by default), the layers will be added to the
     *                      legend and to the main canvas. If you have a private
     *                      layer, you can set this parameter to false to hide it.
     * @param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here, you have take care of deleting
     *                      the layers yourself. Not available in python.
     *
     * @return QList<QgsMapLayer *> - a list of the map layers that were added
     *         successfully. If a layer is invalid, or already exists in the registry,
     *         it will not be part of the returned QList.
     *
     * @note As a side-effect QgsProject is made dirty.
     * @note takeOwner not available in python binding - always takes ownership
     */
    QList<QgsMapLayer *> addMapLayers( QList<QgsMapLayer *> theMapLayers,
                                       bool addToLegend = true,
                                       bool takeOwnership = true );

    /**
     * @brief
     * Add a layer to the map of loaded layers
     *
     * The layersAdded() and layersWasAdded() signals will be emitted in any case.
     * The legendLayersAdded() signal only if addToLegend is true.
     * If you are adding multiple layers at once, you should use
     * {@link addMapLayers()} instead.
     *
     * @param theMapLayer  A layer to add to the registry
     * @param addToLegend If true (by default), the layer will be added to the
     *                    legend and to the main canvas. If you have a private
     *                    you can set this parameter to false to hide it.
     * @param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here, you have take care of deleting
     *                      the layer yourself. Not available in python.
     *
     * @return NULL if unable to add layer, otherwise pointer to newly added layer
     *
     * @see addMapLayers
     *
     * @note As a side-effect QgsProject is made dirty.
     * @note Use addMapLayers if adding more than one layer at a time
     * @note takeOwner not available in python binding - always takes ownership
     */
    QgsMapLayer* addMapLayer( QgsMapLayer * theMapLayer, bool addToLegend = true, bool takeOwnership = true );

    /**
     * @brief
     * Remove a set of layers from the registry
     *
     * Any canvases using the affected layers will need to remove them
     *
     * The layers being removed are deleted as well as the registry
     * table entries.
     *
     * @param theLayerIds  The ids of the layers to remove
     *
     * @note As a side-effect QgsProject is made dirty.
     */
    void removeMapLayers( QStringList theLayerIds );

    /**
     * @brief
     * Remove a layer from qgis
     *
     * Any canvases using the affected layers will need to remove them
     *
     * The layer being removed is deleted as well as the registry
     * table entry.
     *
     * @param theLayerId   The id of the layer to remove
     *
     * @note As a side-effect QgsProject is made dirty.
     */
    void removeMapLayer( const QString& theLayerId );

    /**
     * Remove all registered layers
     *
     * @note As a side-effect QgsProject is made dirty.
     * @note The layers are deleted as the registry is cleared!
     */
    void removeAllMapLayers();

    /**
     * Clears all layer caches, resetting them to zero and
     * freeing up any memory they may have been using. Layer
     * caches are used to speed up rendering in certain situations
     * see ticket #1974 for more details.
     */
    //! @deprecated since 2.4 - does nothing
    Q_DECL_DEPRECATED void clearAllLayerCaches();

    /**
     * Reload all provider data caches (currently used for WFS and WMS providers)
     */
    void reloadAllLayers();

  signals:
    /**
     * Emitted when one or more layers are removed from the registry
     *
     * @param theLayerIds  A list of ids of the layers which are removed.
     */
    void layersWillBeRemoved( QStringList theLayerIds );

    /**
     * Emitted when a layer is removed from the registry
     *
     * @param theLayerId  The id of the layer being removed
     *
     * @note Consider using {@link layersWillBeRemoved()} instead
     */
    void layerWillBeRemoved( QString theLayerId );

    /**
     * Emitted after one or more layers were removed from the registry
     *
     * @param theLayerIds  A list of ids of the layers which were removed.
     */
    void layersRemoved( QStringList theLayerIds );

    /**
     * Emitted after a layer was removed from the registry
     *
     * @param theLayerId  The id of the layer removed
     *
     * @note Consider using {@link layersRemoved()} instead
     */
    void layerRemoved( QString theLayerId );


    /**
     * Emitted, when all layers are removed, before {@link layersWillBeRemoved()} and
     * {@link layerWillBeRemoved()} signals are emitted. You will still get these signals
     * in any case.
     * You can use this signal to do easy (and fast) cleanup.
     */
    void removeAll();

    /**
     * Emitted when one or more layers are added to the registry.
     * This signal is also emitted for layers added to the registry,
     * but not to the legend and canvas.
     *
     * @param theMapLayers  The layers which have been added
     *
     * @see legendLayersAdded()
     */
    void layersAdded( QList<QgsMapLayer *> theMapLayers );

    /**
     * Emitted when a layer is added to the registry.
     *
     * @param theMapLayer  The id of the layer which has been added
     *
     * @note Consider using {@link layersAdded()} instead
     */
    void layerWasAdded( QgsMapLayer* theMapLayer );

    /**
     * Emitted, when a layer is added to the registry and the legend.
     * Plugins are allowed to have private layers, which are signalled by
     * {@link layersAdded()} and {@link layerWasAdded()} but will not be
     * advertised by this signal.
     *
     * @param theMapLayers  The {@link QgsMapLayer}s which are added to the legend.
     */
    void legendLayersAdded( QList<QgsMapLayer*> theMapLayers );

  protected:
#if 0
    /** debugging member
        invoked when a connect() is made to this object
    */
    void connectNotify( const char * signal ) override;
#endif

  private:
    //! private singleton constructor
    QgsMapLayerRegistry( QObject * parent = 0 );

    QMap<QString, QgsMapLayer*> mMapLayers;
    QSet<QgsMapLayer*> mOwnedLayers;
}; // class QgsMapLayerRegistry

#endif //QgsMapLayerRegistry_H

