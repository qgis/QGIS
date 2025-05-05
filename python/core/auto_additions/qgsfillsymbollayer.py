# The following has been generated automatically from src/core/symbology/qgsfillsymbollayer.h
try:
    QgsSimpleFillSymbolLayer.create = staticmethod(QgsSimpleFillSymbolLayer.create)
    QgsSimpleFillSymbolLayer.createFromSld = staticmethod(QgsSimpleFillSymbolLayer.createFromSld)
    QgsSimpleFillSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'ogrFeatureStyle', 'strokeColor', 'setStrokeColor', 'fillColor', 'setFillColor', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'estimateMaxBleed', 'dxfWidth', 'dxfColor', 'dxfAngle', 'dxfPenStyle', 'dxfBrushColor', 'dxfBrushStyle', 'toTiledPatternImage']
    QgsSimpleFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsGradientFillSymbolLayer.create = staticmethod(QgsGradientFillSymbolLayer.create)
    QgsGradientFillSymbolLayer.__overridden_methods__ = ['flags', 'layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'estimateMaxBleed', 'canCauseArtifactsBetweenAdjacentTiles', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale']
    import functools as _functools
    __wrapped_QgsGradientFillSymbolLayer_setColorRamp = QgsGradientFillSymbolLayer.setColorRamp
    def __QgsGradientFillSymbolLayer_setColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGradientFillSymbolLayer_setColorRamp(self, arg)
    QgsGradientFillSymbolLayer.setColorRamp = _functools.update_wrapper(__QgsGradientFillSymbolLayer_setColorRamp_wrapper, QgsGradientFillSymbolLayer.setColorRamp)

    QgsGradientFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsShapeburstFillSymbolLayer.create = staticmethod(QgsShapeburstFillSymbolLayer.create)
    QgsShapeburstFillSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'estimateMaxBleed', 'canCauseArtifactsBetweenAdjacentTiles', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale']
    import functools as _functools
    __wrapped_QgsShapeburstFillSymbolLayer_setColorRamp = QgsShapeburstFillSymbolLayer.setColorRamp
    def __QgsShapeburstFillSymbolLayer_setColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsShapeburstFillSymbolLayer_setColorRamp(self, arg)
    QgsShapeburstFillSymbolLayer.setColorRamp = _functools.update_wrapper(__QgsShapeburstFillSymbolLayer_setColorRamp_wrapper, QgsShapeburstFillSymbolLayer.setColorRamp)

    QgsShapeburstFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRasterFillSymbolLayer.create = staticmethod(QgsRasterFillSymbolLayer.create)
    QgsRasterFillSymbolLayer.createFromSld = staticmethod(QgsRasterFillSymbolLayer.createFromSld)
    QgsRasterFillSymbolLayer.resolvePaths = staticmethod(QgsRasterFillSymbolLayer.resolvePaths)
    QgsRasterFillSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'renderPolygon', 'startRender', 'stopRender', 'properties', 'clone', 'estimateMaxBleed', 'usesMapUnits', 'color', 'setOutputUnit', 'subSymbol', 'setSubSymbol', 'applyDataDefinedSettings', 'applyBrushTransformFromContext']
    import functools as _functools
    __wrapped_QgsRasterFillSymbolLayer_setSubSymbol = QgsRasterFillSymbolLayer.setSubSymbol
    def __QgsRasterFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterFillSymbolLayer_setSubSymbol(self, arg)
    QgsRasterFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsRasterFillSymbolLayer_setSubSymbol_wrapper, QgsRasterFillSymbolLayer.setSubSymbol)

    QgsRasterFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSVGFillSymbolLayer.create = staticmethod(QgsSVGFillSymbolLayer.create)
    QgsSVGFillSymbolLayer.createFromSld = staticmethod(QgsSVGFillSymbolLayer.createFromSld)
    QgsSVGFillSymbolLayer.resolvePaths = staticmethod(QgsSVGFillSymbolLayer.resolvePaths)
    QgsSVGFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'usesMapUnits', 'subSymbol', 'setSubSymbol', 'estimateMaxBleed', 'dxfColor', 'usedAttributes', 'hasDataDefinedProperties', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'applyDataDefinedSettings']
    import functools as _functools
    __wrapped_QgsSVGFillSymbolLayer_setSubSymbol = QgsSVGFillSymbolLayer.setSubSymbol
    def __QgsSVGFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSVGFillSymbolLayer_setSubSymbol(self, arg)
    QgsSVGFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsSVGFillSymbolLayer_setSubSymbol_wrapper, QgsSVGFillSymbolLayer.setSubSymbol)

    QgsSVGFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsLinePatternFillSymbolLayer.create = staticmethod(QgsLinePatternFillSymbolLayer.create)
    QgsLinePatternFillSymbolLayer.createFromSld = staticmethod(QgsLinePatternFillSymbolLayer.createFromSld)
    QgsLinePatternFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'toTiledPatternImage', 'estimateMaxBleed', 'setColor', 'color', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'setSubSymbol', 'subSymbol', 'usedAttributes', 'hasDataDefinedProperties', 'startFeatureRender', 'stopFeatureRender', 'applyDataDefinedSettings']
    import functools as _functools
    __wrapped_QgsLinePatternFillSymbolLayer_setSubSymbol = QgsLinePatternFillSymbolLayer.setSubSymbol
    def __QgsLinePatternFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLinePatternFillSymbolLayer_setSubSymbol(self, arg)
    QgsLinePatternFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsLinePatternFillSymbolLayer_setSubSymbol_wrapper, QgsLinePatternFillSymbolLayer.setSubSymbol)

    QgsLinePatternFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsPointPatternFillSymbolLayer.create = staticmethod(QgsPointPatternFillSymbolLayer.create)
    QgsPointPatternFillSymbolLayer.createFromSld = staticmethod(QgsPointPatternFillSymbolLayer.createFromSld)
    QgsPointPatternFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'toTiledPatternImage', 'estimateMaxBleed', 'setSubSymbol', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'setColor', 'color', 'applyDataDefinedSettings']
    import functools as _functools
    __wrapped_QgsPointPatternFillSymbolLayer_setSubSymbol = QgsPointPatternFillSymbolLayer.setSubSymbol
    def __QgsPointPatternFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointPatternFillSymbolLayer_setSubSymbol(self, arg)
    QgsPointPatternFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsPointPatternFillSymbolLayer_setSubSymbol_wrapper, QgsPointPatternFillSymbolLayer.setSubSymbol)

    QgsPointPatternFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRandomMarkerFillSymbolLayer.create = staticmethod(QgsRandomMarkerFillSymbolLayer.create)
    QgsRandomMarkerFillSymbolLayer.__virtual_methods__ = ['setSubSymbol']
    QgsRandomMarkerFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'canCauseArtifactsBetweenAdjacentTiles', 'setColor', 'color', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'startFeatureRender', 'stopFeatureRender']
    import functools as _functools
    __wrapped_QgsRandomMarkerFillSymbolLayer_setSubSymbol = QgsRandomMarkerFillSymbolLayer.setSubSymbol
    def __QgsRandomMarkerFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRandomMarkerFillSymbolLayer_setSubSymbol(self, arg)
    QgsRandomMarkerFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsRandomMarkerFillSymbolLayer_setSubSymbol_wrapper, QgsRandomMarkerFillSymbolLayer.setSubSymbol)

    QgsRandomMarkerFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsCentroidFillSymbolLayer.create = staticmethod(QgsCentroidFillSymbolLayer.create)
    QgsCentroidFillSymbolLayer.createFromSld = staticmethod(QgsCentroidFillSymbolLayer.createFromSld)
    QgsCentroidFillSymbolLayer.__virtual_methods__ = ['setSubSymbol']
    QgsCentroidFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'setColor', 'color', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'canCauseArtifactsBetweenAdjacentTiles', 'startFeatureRender', 'stopFeatureRender']
    import functools as _functools
    __wrapped_QgsCentroidFillSymbolLayer_setSubSymbol = QgsCentroidFillSymbolLayer.setSubSymbol
    def __QgsCentroidFillSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCentroidFillSymbolLayer_setSubSymbol(self, arg)
    QgsCentroidFillSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsCentroidFillSymbolLayer_setSubSymbol_wrapper, QgsCentroidFillSymbolLayer.setSubSymbol)

    QgsCentroidFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsImageFillSymbolLayer.__virtual_methods__ = ['applyDataDefinedSettings', 'applyBrushTransformFromContext']
    QgsImageFillSymbolLayer.__overridden_methods__ = ['renderPolygon', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'dxfWidth', 'dxfPenStyle', 'properties']
    QgsImageFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
