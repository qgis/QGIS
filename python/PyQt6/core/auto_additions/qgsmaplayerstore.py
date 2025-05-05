# The following has been generated automatically from src/core/qgsmaplayerstore.h
try:
    QgsMapLayerStore.__attribute_docs__ = {'layersWillBeRemoved': 'Emitted when one or more layers are about to be removed from the store.\n\n:param layers: A list of layers which are to be removed.\n\n.. seealso:: :py:func:`layerWillBeRemoved`\n\n.. seealso:: :py:func:`layersRemoved`\n', 'layerWillBeRemoved': 'Emitted when a layer is about to be removed from the store.\n\n:param layer: The layer to be removed.\n\n.. note::\n\n   Consider using :py:func:`~QgsMapLayerStore.layersWillBeRemoved` instead.\n\n.. seealso:: :py:func:`layersWillBeRemoved`\n\n.. seealso:: :py:func:`layerRemoved`\n', 'layersRemoved': 'Emitted after one or more layers were removed from the store.\n\n:param layerIds: A list of IDs of the layers which were removed.\n\n.. seealso:: :py:func:`layersWillBeRemoved`\n', 'layerRemoved': 'Emitted after a layer was removed from the store.\n\n:param layerId: The ID of the layer removed.\n\n.. note::\n\n   Consider using :py:func:`~QgsMapLayerStore.layersRemoved` instead\n\n.. seealso:: :py:func:`layerWillBeRemoved`\n', 'allLayersRemoved': 'Emitted when all layers are removed, before\n:py:func:`~QgsMapLayerStore.layersWillBeRemoved` and\n:py:func:`~QgsMapLayerStore.layerWillBeRemoved` signals are emitted. The\n:py:func:`~QgsMapLayerStore.layersWillBeRemoved` and\n:py:func:`~QgsMapLayerStore.layerWillBeRemoved` signals will still be\nemitted following this signal. You can use this signal to do easy (and\nfast) cleanup.\n', 'layersAdded': 'Emitted when one or more layers were added to the store.\n\n:param layers: List of layers which have been added.\n\n.. seealso:: :py:func:`layerWasAdded`\n', 'layerWasAdded': 'Emitted when a ``layer`` was added to the store.\n\n.. note::\n\n   Consider using :py:func:`~QgsMapLayerStore.layersAdded` instead\n\n.. seealso:: :py:func:`layersAdded`\n'}
    QgsMapLayerStore.__signal_arguments__ = {'layersWillBeRemoved': ['layers: List[QgsMapLayer]'], 'layerWillBeRemoved': ['layer: QgsMapLayer'], 'layersRemoved': ['layerIds: List[str]'], 'layerRemoved': ['layerId: str'], 'layersAdded': ['layers: List[QgsMapLayer]'], 'layerWasAdded': ['layer: QgsMapLayer']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMapLayerStore_addMapLayers = QgsMapLayerStore.addMapLayers
    def __QgsMapLayerStore_addMapLayers_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMapLayerStore_addMapLayers(self, arg)
    QgsMapLayerStore.addMapLayers = _functools.update_wrapper(__QgsMapLayerStore_addMapLayers_wrapper, QgsMapLayerStore.addMapLayers)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMapLayerStore_addMapLayer = QgsMapLayerStore.addMapLayer
    def __QgsMapLayerStore_addMapLayer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMapLayerStore_addMapLayer(self, arg)
    QgsMapLayerStore.addMapLayer = _functools.update_wrapper(__QgsMapLayerStore_addMapLayer_wrapper, QgsMapLayerStore.addMapLayer)

except (NameError, AttributeError):
    pass
