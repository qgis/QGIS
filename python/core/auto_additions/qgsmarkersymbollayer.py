# The following has been generated automatically from src/core/symbology/qgsmarkersymbollayer.h
try:
    QgsSimpleMarkerSymbolLayerBase.availableShapes = staticmethod(QgsSimpleMarkerSymbolLayerBase.availableShapes)
    QgsSimpleMarkerSymbolLayerBase.shapeIsFilled = staticmethod(QgsSimpleMarkerSymbolLayerBase.shapeIsFilled)
    QgsSimpleMarkerSymbolLayerBase.decodeShape = staticmethod(QgsSimpleMarkerSymbolLayerBase.decodeShape)
    QgsSimpleMarkerSymbolLayerBase.encodeShape = staticmethod(QgsSimpleMarkerSymbolLayerBase.encodeShape)
    QgsSimpleMarkerSymbolLayerBase.__abstract_methods__ = ['draw']
    QgsSimpleMarkerSymbolLayerBase.__overridden_methods__ = ['startRender', 'stopRender', 'renderPoint', 'bounds']
    QgsSimpleMarkerSymbolLayerBase.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSimpleMarkerSymbolLayer.create = staticmethod(QgsSimpleMarkerSymbolLayer.create)
    QgsSimpleMarkerSymbolLayer.createFromSld = staticmethod(QgsSimpleMarkerSymbolLayer.createFromSld)
    QgsSimpleMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'renderPoint', 'properties', 'clone', 'toSld', 'writeSldMarker', 'ogrFeatureStyle', 'writeDxf', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'usesMapUnits', 'bounds', 'fillColor', 'setFillColor', 'setColor', 'color', 'strokeColor', 'setStrokeColor', 'draw']
    QgsSimpleMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsFilledMarkerSymbolLayer.create = staticmethod(QgsFilledMarkerSymbolLayer.create)
    QgsFilledMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'properties', 'clone', 'subSymbol', 'setSubSymbol', 'estimateMaxBleed', 'usedAttributes', 'hasDataDefinedProperties', 'setColor', 'color', 'usesMapUnits', 'setOutputUnit', 'draw']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFilledMarkerSymbolLayer_setSubSymbol = QgsFilledMarkerSymbolLayer.setSubSymbol
    def __QgsFilledMarkerSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFilledMarkerSymbolLayer_setSubSymbol(self, arg)
    QgsFilledMarkerSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsFilledMarkerSymbolLayer_setSubSymbol_wrapper, QgsFilledMarkerSymbolLayer.setSubSymbol)

    QgsFilledMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSvgMarkerSymbolLayer.create = staticmethod(QgsSvgMarkerSymbolLayer.create)
    QgsSvgMarkerSymbolLayer.createFromSld = staticmethod(QgsSvgMarkerSymbolLayer.createFromSld)
    QgsSvgMarkerSymbolLayer.resolvePaths = staticmethod(QgsSvgMarkerSymbolLayer.resolvePaths)
    QgsSvgMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPoint', 'properties', 'usesMapUnits', 'clone', 'toSld', 'writeSldMarker', 'fillColor', 'setFillColor', 'strokeColor', 'setStrokeColor', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'writeDxf', 'bounds', 'prepareExpressions', 'usedAttributes']
    QgsSvgMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRasterMarkerSymbolLayer.create = staticmethod(QgsRasterMarkerSymbolLayer.create)
    QgsRasterMarkerSymbolLayer.resolvePaths = staticmethod(QgsRasterMarkerSymbolLayer.resolvePaths)
    QgsRasterMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'renderPoint', 'properties', 'clone', 'usesMapUnits', 'color', 'setMapUnitScale', 'mapUnitScale', 'bounds']
    QgsRasterMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsFontMarkerSymbolLayer.create = staticmethod(QgsFontMarkerSymbolLayer.create)
    QgsFontMarkerSymbolLayer.createFromSld = staticmethod(QgsFontMarkerSymbolLayer.createFromSld)
    QgsFontMarkerSymbolLayer.resolveFonts = staticmethod(QgsFontMarkerSymbolLayer.resolveFonts)
    QgsFontMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPoint', 'properties', 'clone', 'toSld', 'writeSldMarker', 'usesMapUnits', 'setOutputUnit', 'strokeColor', 'setStrokeColor', 'bounds']
    QgsFontMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsAnimatedMarkerSymbolLayer.create = staticmethod(QgsAnimatedMarkerSymbolLayer.create)
    QgsAnimatedMarkerSymbolLayer.__overridden_methods__ = ['layerType', 'properties', 'clone', 'startRender']
    QgsAnimatedMarkerSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
