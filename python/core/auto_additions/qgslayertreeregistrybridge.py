# The following has been generated automatically from src/core/layertree/qgslayertreeregistrybridge.h
try:
    QgsLayerTreeRegistryBridge.__attribute_docs__ = {'addedLayersToLayerTree': 'Tell others we have just added layers to the tree (used in QGIS to auto-select first newly added layer)\n'}
    QgsLayerTreeRegistryBridge.__signal_arguments__ = {'addedLayersToLayerTree': ['layers: List[QgsMapLayer]']}
    QgsLayerTreeRegistryBridge.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsLayerTreeRegistryBridge.InsertionPoint.__doc__ = """A structure to define the insertion point to the layer tree.
This represents the current layer tree group and index where newly added map layers should be inserted into.

.. versionadded:: 3.10"""
    QgsLayerTreeRegistryBridge.InsertionPoint.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
