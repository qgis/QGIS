# The following has been generated automatically from src/core/symbology/qgsgeometrygeneratorsymbollayer.h
try:
    QgsGeometryGeneratorSymbolLayer.create = staticmethod(QgsGeometryGeneratorSymbolLayer.create)
    QgsGeometryGeneratorSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'usesMapUnits', 'color', 'outputUnit', 'setOutputUnit', 'mapUnitScale', 'clone', 'properties', 'drawPreviewIcon', 'subSymbol', 'setSubSymbol', 'usedAttributes', 'hasDataDefinedProperties', 'isCompatibleWithSymbol', 'setColor']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGeometryGeneratorSymbolLayer_setSubSymbol = QgsGeometryGeneratorSymbolLayer.setSubSymbol
    def __QgsGeometryGeneratorSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryGeneratorSymbolLayer_setSubSymbol(self, arg)
    QgsGeometryGeneratorSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsGeometryGeneratorSymbolLayer_setSubSymbol_wrapper, QgsGeometryGeneratorSymbolLayer.setSubSymbol)

    QgsGeometryGeneratorSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
