/***************************************************************************
                         qgsmaplayerstore.h
                         ------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMAPLAYERSTORE_H
#define QGSMAPLAYERSTORE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include <QObject>

/**
 * \class QgsMapLayerStore
 * \ingroup core
 * A storage object for map layers, in which the layers are owned by the
 * store and have their lifetime bound to the store.
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsMapLayerStore : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapLayerStore.
     */
    explicit QgsMapLayerStore( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsMapLayerStore() override;

    /**
     * Returns the number of layers contained in the store.
     */
    int count() const;

    /**
     * Returns the number of valid layers contained in the store.
     * \since QGIS 3.6
     */
    int validCount() const;

#ifdef SIP_RUN

    /**
     * Returns the number of layers contained in the store.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->count();
    % End

    //! Ensures that bool(obj) returns true (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End
#endif

    /**
     * Retrieve a pointer to a layer by layer \a id.
     * \param id ID of layer to retrieve
     * \returns matching layer, or nullptr if no matching layer found
     * \see mapLayersByName()
     * \see mapLayers()
     */
    QgsMapLayer *mapLayer( const QString &id ) const;

    /**
     * Retrieve a list of matching layers by layer \a name.
     * \param name name of layers to match
     * \returns list of matching layers
     * \see mapLayer()
     * \see mapLayers()
     */
    QList<QgsMapLayer *> mapLayersByName( const QString &name ) const;

    /**
     * Returns a map of all layers by layer ID.
     * \see mapLayer()
     * \see mapLayersByName()
     * \see layers()
     */
    QMap<QString, QgsMapLayer *> mapLayers() const;

    /**
     * Returns a map of all valid layers by layer ID.
     * \see mapLayer()
     * \see mapLayersByName()
     * \see layers()
     * \since QGIS 3.6
     */
    QMap<QString, QgsMapLayer *> validMapLayers() const;

#ifndef SIP_RUN

    /**
     * Returns a list of registered map layers with a specified layer type.
     *
     * Example:
     *
     *     QVector<QgsVectorLayer*> vectorLayers = store->layers<QgsVectorLayer*>();
     *
     * \note not available in Python bindings
     * \see mapLayers()
     */
    template <typename T>
    QVector<T> layers() const
    {
      QVector<T> layers;
      QMap<QString, QgsMapLayer *>::const_iterator layerIt = mMapLayers.constBegin();
      for ( ; layerIt != mMapLayers.constEnd(); ++layerIt )
      {
        T tLayer = qobject_cast<T>( layerIt.value() );
        if ( tLayer )
        {
          layers << tLayer;
        }
      }
      return layers;
    }
#endif

    /**
     * \brief
     * Add a list of \a layers to the store. Ownership of the layers is transferred
     * to the store.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     *
     * \param layers A list of layer which should be added to the store.
     * \param takeOwnership Ownership will be transferred to the layer store.
     *                      If you specify false here you have take care of deleting
     *                      the layers yourself. Not available in Python.
     *
     * \returns a list of the map layers that were added
     *         successfully. If a layer already exists in the store,
     *         it will not be part of the returned list.
     *
     * \see addMapLayer()
     */
    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *> &layers SIP_TRANSFER,
                                       bool takeOwnership SIP_PYARGREMOVE = true );

    /**
     * \brief
     * Add a \a layer to the store. Ownership of the layer is transferred to the
     * store.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * If you are adding multiple layers at once, you should use
     * addMapLayers() instead.
     *
     * \param layer A layer to add to the store
     * \param takeOwnership Ownership will be transferred to the layer store.
     *                      If you specify false here you have take care of deleting
     *                      the layers yourself. Not available in Python.
     *
     * \returns nullptr if unable to add layer, otherwise pointer to newly added layer
     *
     * \see addMapLayers
     *
     * \note Use addMapLayers() if adding more than one layer at a time.
     * \see addMapLayers()
     */
    QgsMapLayer *addMapLayer( QgsMapLayer *layer SIP_TRANSFER,
                              bool takeOwnership SIP_PYARGREMOVE = true );

    /**
     * \brief
     * Remove a set of layers from the store by layer ID.
     *
     * The specified layers will be removed from the store.
     * These layers will also be deleted.
     *
     * \param layerIds list of IDs of the layers to remove
     *
     * \see takeMapLayer()
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     * \note available in Python bindings as removeMapLayersById.
     */
    void removeMapLayers( const QStringList &layerIds ) SIP_PYNAME( removeMapLayersById );

    /**
     * \brief
     * Remove a set of \a layers from the store.
     *
     * The specified layers will be removed from the store.
     * These layers will also be deleted.
     *
     * \param layers A list of layers to remove. Null pointers are ignored.
     *
     * \see takeMapLayer()
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     */
    void removeMapLayers( const QList<QgsMapLayer *> &layers );

    /**
     * \brief
     * Remove a layer from the store by layer \a id.
     *
     * The specified layer will be removed from the store. The layer will also be deleted.
     *
     * \param id ID of the layer to remove
     *
     * \see takeMapLayer()
     * \see removeMapLayers()
     * \see removeAllMapLayers()
     */
    void removeMapLayer( const QString &id );

    /**
     * \brief
     * Remove a \a layer from the store.
     *
     * The specified layer will be removed from the store. The layer will also be deleted.
     *
     * \param layer The layer to remove. Null pointers are ignored.
     *
     * \see takeMapLayer()
     * \see removeMapLayers()
     * \see removeAllMapLayers()
     */
    void removeMapLayer( QgsMapLayer *layer );

    /**
     * Takes a \a layer from the store. If the layer was owned by the store, the
     * layer will be returned without deleting it. The caller takes ownership of
     * the layer and is responsible for deleting it.
     * \see removeMapLayer()
     */
    QgsMapLayer *takeMapLayer( QgsMapLayer *layer ) SIP_TRANSFERBACK;

    /**
     * Removes all registered layers. These layers will also be deleted.
     *
     * \note Calling this method will cause the removeAll() signal to
     * be emitted.
     * \see removeMapLayer()
     * \see removeMapLayers()
     */
    void removeAllMapLayers();

    /**
     * Transfers all the map layers contained within another map layer store and adds
     * them to this store.
     * Note that \a other and this store must have the same thread affinity.
     */
    void transferLayersFromStore( QgsMapLayerStore *other );

  signals:

    /**
     * Emitted when one or more layers are about to be removed from the store.
     *
     * \param layerIds A list of IDs for the layers which are to be removed.
     * \see layerWillBeRemoved()
     * \see layersRemoved()
     */
    void layersWillBeRemoved( const QStringList &layerIds );

    /**
     * Emitted when one or more layers are about to be removed from the store.
     *
     * \param layers A list of layers which are to be removed.
     * \see layerWillBeRemoved()
     * \see layersRemoved()
     */
    void layersWillBeRemoved( const QList<QgsMapLayer *> &layers );

    /**
     * Emitted when a layer is about to be removed from the store.
     *
     * \param layerId The ID of the layer to be removed.
     *
     * \note Consider using layersWillBeRemoved() instead.
     * \see layersWillBeRemoved()
     * \see layerRemoved()
     */
    void layerWillBeRemoved( const QString &layerId );

    /**
     * Emitted when a layer is about to be removed from the store.
     *
     * \param layer The layer to be removed.
     *
     * \note Consider using layersWillBeRemoved() instead.
     * \see layersWillBeRemoved()
     * \see layerRemoved()
     */
    void layerWillBeRemoved( QgsMapLayer *layer );

    /**
     * Emitted after one or more layers were removed from the store.
     *
     * \param layerIds  A list of IDs of the layers which were removed.
     * \see layersWillBeRemoved()
     */
    void layersRemoved( const QStringList &layerIds );

    /**
     * Emitted after a layer was removed from the store.
     *
     * \param layerId The ID of the layer removed.
     *
     * \note Consider using layersRemoved() instead
     * \see layerWillBeRemoved()
     */
    void layerRemoved( const QString &layerId );

    /**
     * Emitted when all layers are removed, before layersWillBeRemoved() and
     * layerWillBeRemoved() signals are emitted. The layersWillBeRemoved() and
     * layerWillBeRemoved() signals will still be emitted following this signal.
     * You can use this signal to do easy (and fast) cleanup.
     */
    void allLayersRemoved();

    /**
     * Emitted when one or more layers were added to the store.
     *
     * \param layers List of layers which have been added.
     *
     * \see layerWasAdded()
     */
    void layersAdded( const QList<QgsMapLayer *> &layers );

    /**
     * Emitted when a \a layer was added to the store.
     *
     * \note Consider using layersAdded() instead
     * \see layersAdded()
     */
    void layerWasAdded( QgsMapLayer *layer );

  private slots:

    void onMapLayerDeleted( QObject *obj );

  private:

    QMap<QString, QgsMapLayer *> mMapLayers;

};

#endif //QGSMAPLAYERSTORE_H
