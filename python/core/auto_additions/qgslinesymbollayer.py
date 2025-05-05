# The following has been generated automatically from src/core/symbology/qgslinesymbollayer.h
try:
    QgsSimpleLineSymbolLayer.create = staticmethod(QgsSimpleLineSymbolLayer.create)
    QgsSimpleLineSymbolLayer.createFromSld = staticmethod(QgsSimpleLineSymbolLayer.createFromSld)
    QgsSimpleLineSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolyline', 'renderPolygonStroke', 'properties', 'clone', 'toSld', 'ogrFeatureStyle', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'estimateMaxBleed', 'dxfCustomDashPattern', 'dxfPenStyle', 'dxfWidth', 'dxfOffset', 'dxfColor', 'canCauseArtifactsBetweenAdjacentTiles']
    QgsSimpleLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsTemplatedLineSymbolLayerBase.setCommonProperties = staticmethod(QgsTemplatedLineSymbolLayerBase.setCommonProperties)
    QgsTemplatedLineSymbolLayerBase.__virtual_methods__ = ['renderPolygonStroke', 'outputUnit', 'setMapUnitScale', 'mapUnitScale']
    QgsTemplatedLineSymbolLayerBase.__abstract_methods__ = ['setSymbolLineAngle', 'symbolAngle', 'setSymbolAngle', 'renderSymbol']
    QgsTemplatedLineSymbolLayerBase.__overridden_methods__ = ['renderPolyline', 'setOutputUnit', 'properties', 'canCauseArtifactsBetweenAdjacentTiles', 'startFeatureRender', 'stopFeatureRender']
    QgsTemplatedLineSymbolLayerBase.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsMarkerLineSymbolLayer.create = staticmethod(QgsMarkerLineSymbolLayer.create)
    QgsMarkerLineSymbolLayer.createFromSld = staticmethod(QgsMarkerLineSymbolLayer.createFromSld)
    QgsMarkerLineSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'clone', 'toSld', 'setColor', 'color', 'subSymbol', 'setSubSymbol', 'setWidth', 'width', 'estimateMaxBleed', 'setOutputUnit', 'usesMapUnits', 'usedAttributes', 'hasDataDefinedProperties', 'setDataDefinedProperty', 'renderPolyline', 'setSymbolLineAngle', 'symbolAngle', 'setSymbolAngle', 'renderSymbol']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMarkerLineSymbolLayer_setSubSymbol = QgsMarkerLineSymbolLayer.setSubSymbol
    def __QgsMarkerLineSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMarkerLineSymbolLayer_setSubSymbol(self, arg)
    QgsMarkerLineSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsMarkerLineSymbolLayer_setSubSymbol_wrapper, QgsMarkerLineSymbolLayer.setSubSymbol)

    QgsMarkerLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsHashedLineSymbolLayer.create = staticmethod(QgsHashedLineSymbolLayer.create)
    QgsHashedLineSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'properties', 'clone', 'setColor', 'color', 'subSymbol', 'setSubSymbol', 'setWidth', 'width', 'estimateMaxBleed', 'setOutputUnit', 'usedAttributes', 'hasDataDefinedProperties', 'setDataDefinedProperty', 'usesMapUnits', 'renderPolyline', 'setSymbolLineAngle', 'symbolAngle', 'setSymbolAngle', 'renderSymbol']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsHashedLineSymbolLayer_setSubSymbol = QgsHashedLineSymbolLayer.setSubSymbol
    def __QgsHashedLineSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsHashedLineSymbolLayer_setSubSymbol(self, arg)
    QgsHashedLineSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsHashedLineSymbolLayer_setSubSymbol_wrapper, QgsHashedLineSymbolLayer.setSubSymbol)

    QgsHashedLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRasterLineSymbolLayer.create = staticmethod(QgsRasterLineSymbolLayer.create)
    QgsRasterLineSymbolLayer.resolvePaths = staticmethod(QgsRasterLineSymbolLayer.resolvePaths)
    QgsRasterLineSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolyline', 'properties', 'clone', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'estimateMaxBleed', 'color']
    QgsRasterLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsLineburstSymbolLayer.create = staticmethod(QgsLineburstSymbolLayer.create)
    QgsLineburstSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolyline', 'properties', 'clone', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'estimateMaxBleed']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLineburstSymbolLayer_setColorRamp = QgsLineburstSymbolLayer.setColorRamp
    def __QgsLineburstSymbolLayer_setColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLineburstSymbolLayer_setColorRamp(self, arg)
    QgsLineburstSymbolLayer.setColorRamp = _functools.update_wrapper(__QgsLineburstSymbolLayer_setColorRamp_wrapper, QgsLineburstSymbolLayer.setColorRamp)

    QgsLineburstSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsFilledLineSymbolLayer.create = staticmethod(QgsFilledLineSymbolLayer.create)
    QgsFilledLineSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'renderPolyline', 'properties', 'clone', 'subSymbol', 'setSubSymbol', 'hasDataDefinedProperties', 'setColor', 'color', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'estimateMaxBleed', 'usedAttributes']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFilledLineSymbolLayer_setSubSymbol = QgsFilledLineSymbolLayer.setSubSymbol
    def __QgsFilledLineSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFilledLineSymbolLayer_setSubSymbol(self, arg)
    QgsFilledLineSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsFilledLineSymbolLayer_setSubSymbol_wrapper, QgsFilledLineSymbolLayer.setSubSymbol)

    QgsFilledLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractBrushedLineSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
