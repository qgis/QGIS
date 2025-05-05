# The following has been generated automatically from src/core/symbology/qgsmasksymbollayer.h
try:
    QgsMaskMarkerSymbolLayer.create = staticmethod(QgsMaskMarkerSymbolLayer.create)
    QgsMaskMarkerSymbolLayer.__overridden_methods__ = ['clone', 'subSymbol', 'setSubSymbol', 'usedAttributes', 'hasDataDefinedProperties', 'properties', 'layerType', 'startRender', 'stopRender', 'renderPoint', 'bounds', 'usesMapUnits', 'setOutputUnit', 'color', 'drawPreviewIcon', 'masks']
    import functools as _functools
    __wrapped_QgsMaskMarkerSymbolLayer_setSubSymbol = QgsMaskMarkerSymbolLayer.setSubSymbol
    def __QgsMaskMarkerSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMaskMarkerSymbolLayer_setSubSymbol(self, arg)
    QgsMaskMarkerSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsMaskMarkerSymbolLayer_setSubSymbol_wrapper, QgsMaskMarkerSymbolLayer.setSubSymbol)

    QgsMaskMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
