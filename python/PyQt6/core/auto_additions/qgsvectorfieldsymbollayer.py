# The following has been generated automatically from src/core/symbology/qgsvectorfieldsymbollayer.h
QgsVectorFieldSymbolLayer.Cartesian = QgsVectorFieldSymbolLayer.VectorFieldType.Cartesian
QgsVectorFieldSymbolLayer.Polar = QgsVectorFieldSymbolLayer.VectorFieldType.Polar
QgsVectorFieldSymbolLayer.Height = QgsVectorFieldSymbolLayer.VectorFieldType.Height
QgsVectorFieldSymbolLayer.ClockwiseFromNorth = QgsVectorFieldSymbolLayer.AngleOrientation.ClockwiseFromNorth
QgsVectorFieldSymbolLayer.CounterclockwiseFromEast = QgsVectorFieldSymbolLayer.AngleOrientation.CounterclockwiseFromEast
QgsVectorFieldSymbolLayer.Degrees = QgsVectorFieldSymbolLayer.AngleUnits.Degrees
QgsVectorFieldSymbolLayer.Radians = QgsVectorFieldSymbolLayer.AngleUnits.Radians
try:
    QgsVectorFieldSymbolLayer.create = staticmethod(QgsVectorFieldSymbolLayer.create)
    QgsVectorFieldSymbolLayer.createFromSld = staticmethod(QgsVectorFieldSymbolLayer.createFromSld)
    QgsVectorFieldSymbolLayer.__overridden_methods__ = ['layerType', 'setSubSymbol', 'subSymbol', 'setColor', 'color', 'renderPoint', 'startRender', 'stopRender', 'clone', 'properties', 'usesMapUnits', 'toSld', 'drawPreviewIcon', 'usedAttributes', 'hasDataDefinedProperties', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'bounds']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsVectorFieldSymbolLayer_setSubSymbol = QgsVectorFieldSymbolLayer.setSubSymbol
    def __QgsVectorFieldSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorFieldSymbolLayer_setSubSymbol(self, arg)
    QgsVectorFieldSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsVectorFieldSymbolLayer_setSubSymbol_wrapper, QgsVectorFieldSymbolLayer.setSubSymbol)

    QgsVectorFieldSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
