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
    QgsGradientFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsShapeburstFillSymbolLayer.create = staticmethod(QgsShapeburstFillSymbolLayer.create)
    QgsShapeburstFillSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'estimateMaxBleed', 'canCauseArtifactsBetweenAdjacentTiles', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale']
    QgsShapeburstFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRasterFillSymbolLayer.create = staticmethod(QgsRasterFillSymbolLayer.create)
    QgsRasterFillSymbolLayer.createFromSld = staticmethod(QgsRasterFillSymbolLayer.createFromSld)
    QgsRasterFillSymbolLayer.resolvePaths = staticmethod(QgsRasterFillSymbolLayer.resolvePaths)
    QgsRasterFillSymbolLayer.__overridden_methods__ = ['layerType', 'flags', 'renderPolygon', 'startRender', 'stopRender', 'properties', 'clone', 'estimateMaxBleed', 'usesMapUnits', 'color', 'setOutputUnit', 'subSymbol', 'setSubSymbol', 'applyDataDefinedSettings', 'applyBrushTransformFromContext']
    QgsRasterFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSVGFillSymbolLayer.create = staticmethod(QgsSVGFillSymbolLayer.create)
    QgsSVGFillSymbolLayer.createFromSld = staticmethod(QgsSVGFillSymbolLayer.createFromSld)
    QgsSVGFillSymbolLayer.resolvePaths = staticmethod(QgsSVGFillSymbolLayer.resolvePaths)
    QgsSVGFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'usesMapUnits', 'subSymbol', 'setSubSymbol', 'estimateMaxBleed', 'dxfColor', 'usedAttributes', 'hasDataDefinedProperties', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'applyDataDefinedSettings']
    QgsSVGFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsLinePatternFillSymbolLayer.create = staticmethod(QgsLinePatternFillSymbolLayer.create)
    QgsLinePatternFillSymbolLayer.createFromSld = staticmethod(QgsLinePatternFillSymbolLayer.createFromSld)
    QgsLinePatternFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'toTiledPatternImage', 'estimateMaxBleed', 'setColor', 'color', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'setSubSymbol', 'subSymbol', 'usedAttributes', 'hasDataDefinedProperties', 'startFeatureRender', 'stopFeatureRender', 'applyDataDefinedSettings']
    QgsLinePatternFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsPointPatternFillSymbolLayer.create = staticmethod(QgsPointPatternFillSymbolLayer.create)
    QgsPointPatternFillSymbolLayer.createFromSld = staticmethod(QgsPointPatternFillSymbolLayer.createFromSld)
    QgsPointPatternFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'startFeatureRender', 'stopFeatureRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'toTiledPatternImage', 'estimateMaxBleed', 'setSubSymbol', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'setColor', 'color', 'applyDataDefinedSettings']
    QgsPointPatternFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRandomMarkerFillSymbolLayer.create = staticmethod(QgsRandomMarkerFillSymbolLayer.create)
    QgsRandomMarkerFillSymbolLayer.__virtual_methods__ = ['setSubSymbol']
    QgsRandomMarkerFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'canCauseArtifactsBetweenAdjacentTiles', 'setColor', 'color', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'startFeatureRender', 'stopFeatureRender']
    QgsRandomMarkerFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsCentroidFillSymbolLayer.create = staticmethod(QgsCentroidFillSymbolLayer.create)
    QgsCentroidFillSymbolLayer.createFromSld = staticmethod(QgsCentroidFillSymbolLayer.createFromSld)
    QgsCentroidFillSymbolLayer.__virtual_methods__ = ['setSubSymbol']
    QgsCentroidFillSymbolLayer.__overridden_methods__ = ['layerType', 'startRender', 'stopRender', 'renderPolygon', 'properties', 'clone', 'toSld', 'setColor', 'color', 'subSymbol', 'setOutputUnit', 'outputUnit', 'usesMapUnits', 'setMapUnitScale', 'mapUnitScale', 'usedAttributes', 'hasDataDefinedProperties', 'canCauseArtifactsBetweenAdjacentTiles', 'startFeatureRender', 'stopFeatureRender']
    QgsCentroidFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsImageFillSymbolLayer.__virtual_methods__ = ['applyDataDefinedSettings', 'applyBrushTransformFromContext']
    QgsImageFillSymbolLayer.__overridden_methods__ = ['renderPolygon', 'setOutputUnit', 'outputUnit', 'setMapUnitScale', 'mapUnitScale', 'dxfWidth', 'dxfPenStyle', 'properties']
    QgsImageFillSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
