# The following has been generated automatically from src/core/symbology/qgssymbollayerregistry.h
try:
    QgsSymbolLayerRegistry.defaultSymbolLayer = staticmethod(QgsSymbolLayerRegistry.defaultSymbolLayer)
    import functools as _functools
    __wrapped_QgsSymbolLayerRegistry_addSymbolLayerType = QgsSymbolLayerRegistry.addSymbolLayerType
    def __QgsSymbolLayerRegistry_addSymbolLayerType_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbolLayerRegistry_addSymbolLayerType(self, arg)
    QgsSymbolLayerRegistry.addSymbolLayerType = _functools.update_wrapper(__QgsSymbolLayerRegistry_addSymbolLayerType_wrapper, QgsSymbolLayerRegistry.addSymbolLayerType)

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
