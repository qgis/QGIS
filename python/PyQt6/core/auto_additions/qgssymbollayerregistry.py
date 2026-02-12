# The following has been generated automatically from src/core/symbology/qgssymbollayerregistry.h
try:
    QgsSymbolLayerRegistry.defaultSymbolLayer = staticmethod(QgsSymbolLayerRegistry.defaultSymbolLayer)
    QgsSymbolLayerRegistry.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLayerAbstractMetadata.__virtual_methods__ = ['createSymbolLayerWidget', 'createSymbolLayerFromSld', 'resolvePaths', 'resolveFonts']
    QgsSymbolLayerAbstractMetadata.__abstract_methods__ = ['createSymbolLayer']
    QgsSymbolLayerAbstractMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLayerMetadata.__overridden_methods__ = ['createSymbolLayer', 'createSymbolLayerWidget', 'createSymbolLayerFromSld', 'resolvePaths', 'resolveFonts']
    QgsSymbolLayerMetadata.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
