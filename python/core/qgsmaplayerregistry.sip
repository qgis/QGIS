/** \ingroup core
 * \class QgsMapLayerRegistry
 * This class tracks map layers that are currently loaded and provides
 * various methods to retrieve matching layers from the registry.
 */
class QgsMapLayerRegistry : QObject
{
%TypeHeaderCode
#include <qgsmaplayerregistry.h>
%End

  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsMapLayerRegistry * instance();

    ~QgsMapLayerRegistry();

    //! Returns the number of registered layers.
    int count() const;

    /** Retrieve a pointer to a registered layer by layer ID.
     * @param theLayerId ID of layer to retrieve
     * @returns matching layer, or nullptr if no matching layer found
     * @see mapLayersByName()
     * @see mapLayers()
     */
    //TODO QGIS 3.0 - rename theLayerId to layerId
    QgsMapLayer* mapLayer( const QString& theLayerId ) const;

    /** Retrieve a list of matching registered layers by layer name.
     * @param layerName name of layers to match
     * @returns list of matching layers
     * @see mapLayer()
     * @see mapLayers()
     */
    QList<QgsMapLayer *> mapLayersByName( const QString& layerName ) const;

    /** Returns a map of all registered layers by layer ID.
     * @see mapLayer()
     * @see mapLayersByName()
     * @see layers()
     */
    QMap<QString, QgsMapLayer*> mapLayers() const;

    /**
     * @brief
     * Add a list of layers to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is true.
     *
     * @param theMapLayers  A list of layer which should be added to the registry
     * @param addToLegend   If true (by default), the layers will be added to the
     *                      legend and to the main canvas. If you have a private
     *                      layer you can set this parameter to false to hide it.
     * @param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here you have take care of deleting
     *                      the layers yourself. Not available in python.
     *
     * @return a list of the map layers that were added
     *         successfully. If a layer is invalid, or already exists in the registry,
     *         it will not be part of the returned QList.
     *
     * @note As a side-effect QgsProject is made dirty.
     * @note takeOwnership is not available in the Python bindings - the registry will always
     * take ownership
     * @note added in QGIS 1.8
     * @see addMapLayer()
     */
    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *>& theMapLayers /Transfer/,
                                       bool addToLegend = true );

    /**
     * @brief
     * Add a layer to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is true.
     * If you are adding multiple layers at once, you should use
     * addMapLayers() instead.
     *
     * @param theMapLayer A layer to add to the registry
     * @param addToLegend If true (by default), the layer will be added to the
     *                    legend and to the main canvas. If you have a private
     *                    layer you can set this parameter to false to hide it.
     * @param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here you have take care of deleting
     *                      the layer yourself. Not available in python.
     *
     * @return nullptr if unable to add layer, otherwise pointer to newly added layer
     *
     * @see addMapLayers
     *
     * @note As a side-effect QgsProject is made dirty.
     * @note Use addMapLayers if adding more than one layer at a time
     * @note takeOwnership is not available in the Python bindings - the registry will always
     * take ownership
     * @see addMapLayers()
     */
    QgsMapLayer* addMapLayer( QgsMapLayer * theMapLayer /Transfer/, bool addToLegend = true );

    /**
     * @brief
     * Remove a set of layers from the registry by layer ID.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * @param theLayerIds list of IDs of the layers to remove
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @note added in QGIS 1.8
     * @see removeMapLayer()
     * @see removeAllMapLayers()
     */
    // TODO QGIS 3.0 - rename theLayerIds to layerIds
    void removeMapLayers( const QStringList& theLayerIds );

    /**
     * @brief
     * Remove a set of layers from the registry.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * @param layers A list of layers to remove. Null pointers are ignored.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @see removeMapLayer()
     * @see removeAllMapLayers()
     */
    void removeMapLayers( const QList<QgsMapLayer*>& layers );

    /**
     * @brief
     * Remove a layer from the registry by layer ID.
     *
     * The specified layer will be removed from the registry. If the registry has ownership
     * of the layer then it will also be deleted.
     *
     * @param theLayerId ID of the layer to remove
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @see removeMapLayers()
     * @see removeAllMapLayers()
     */
    // TODO QGIS 3.0 - rename theLayerId to layerId
    void removeMapLayer( const QString& theLayerId );

    /**
     * @brief
     * Remove a layer from the registry.
     *
     * The specified layer will be removed from the registry. If the registry has ownership
     * of the layer then it will also be deleted.
     *
     * @param layer The layer to remove. Null pointers are ignored.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @see removeMapLayers()
     * @see removeAllMapLayers()
     */
    void removeMapLayer( QgsMapLayer* layer );

    /**
     * Removes all registered layers. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @note Calling this method will cause the removeAll() signal to
     * be emitted.
     * @see removeMapLayer()
     * @see removeMapLayers()
     */
    void removeAllMapLayers();

    /**
     * Clears all layer caches, resetting them to zero and
     * freeing up any memory they may have been using. Layer
     * caches are used to speed up rendering in certain situations
     * see ticket #1974 for more details.
     */
    //! @deprecated since 2.4 - does nothing
    void clearAllLayerCaches() /Deprecated/;

    /**
     * Reload all registered layer's provider data caches, synchronising the layer
     * with any changes in the datasource.
     * @see QgsMapLayer::reload()
     */
    void reloadAllLayers();

  signals:

    /**
     * Emitted when one or more layers are about to be removed from the registry.
     *
     * @param theLayerIds A list of IDs for the layers which are to be removed.
     * @see layerWillBeRemoved()
     * @see layersRemoved()
     */
    // TODO QGIS 3.0 - rename theLayerIds to layerIds
    void layersWillBeRemoved( const QStringList& theLayerIds );

    /**
     * Emitted when one or more layers are about to be removed from the registry.
     *
     * @param layers A list of layers which are to be removed.
     * @see layerWillBeRemoved()
     * @see layersRemoved()
     */
    void layersWillBeRemoved( const QList<QgsMapLayer*>& layers );

    /**
     * Emitted when a layer is about to be removed from the registry.
     *
     * @param theLayerId The ID of the layer to be removed.
     *
     * @note Consider using {@link layersWillBeRemoved()} instead
     * @see layersWillBeRemoved()
     * @see layerRemoved()
     */
    //TODO QGIS 3.0 - rename theLayerId to layerId
    void layerWillBeRemoved( const QString& theLayerId );

    /**
     * Emitted when a layer is about to be removed from the registry.
     *
     * @param layer The layer to be removed.
     *
     * @note Consider using {@link layersWillBeRemoved()} instead
     * @see layersWillBeRemoved()
     * @see layerRemoved()
     */
    void layerWillBeRemoved( QgsMapLayer* layer );

    /**
     * Emitted after one or more layers were removed from the registry.
     *
     * @param theLayerIds  A list of IDs of the layers which were removed.
     * @see layersWillBeRemoved()
     */
    //TODO QGIS 3.0 - rename theLayerIds to layerIds
    void layersRemoved( const QStringList& theLayerIds );

    /**
     * Emitted after a layer was removed from the registry.
     *
     * @param theLayerId The ID of the layer removed.
     *
     * @note Consider using {@link layersRemoved()} instead
     * @see layerWillBeRemoved()
     */
    //TODO QGIS 3.0 - rename theLayerId to layerId
    void layerRemoved( const QString& theLayerId );

    /**
     * Emitted when all layers are removed, before {@link layersWillBeRemoved()} and
     * {@link layerWillBeRemoved()} signals are emitted. The layersWillBeRemoved() and
     * layerWillBeRemoved() signals will still be emitted following this signal.
     * You can use this signal to do easy (and fast) cleanup.
     */
    //TODO QGIS 3.0 - rename to past tense
    void removeAll();

    /**
     * Emitted when one or more layers were added to the registry.
     * This signal is also emitted for layers added to the registry,
     * but not to the legend.
     *
     * @param theMapLayers List of layers which have been added.
     *
     * @see legendLayersAdded()
     * @see layerWasAdded()
     */
    //TODO QGIS 3.0 - rename theMapLayers to mapLayers
    void layersAdded( const QList<QgsMapLayer *>& theMapLayers );

    /**
     * Emitted when a layer was added to the registry.
     *
     * @param theMapLayer The ID of the layer which has been added.
     *
     * @note Consider using {@link layersAdded()} instead
     * @see layersAdded()
     */
    // TODO QGIS 3.0 - rename theMapLayer to layer
    void layerWasAdded( QgsMapLayer* theMapLayer );

    /**
     * Emitted, when a layer was added to the registry and the legend.
     * Layers can also be private layers, which are signalled by
     * {@link layersAdded()} and {@link layerWasAdded()} but will not be
     * advertised by this signal.
     *
     * @param theMapLayers List of {@link QgsMapLayer}s which were added to the legend.
     */
    //TODO QGIS 3.0 rename theMapLayers to mapLayers
    void legendLayersAdded( const QList<QgsMapLayer*>& theMapLayers );

  private:
    //! private singleton constructor
    QgsMapLayerRegistry();

    void connectNotify( const char * signal );
};
