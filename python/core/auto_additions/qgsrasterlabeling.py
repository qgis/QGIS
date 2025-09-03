# The following has been generated automatically from src/core/raster/qgsrasterlabeling.h
try:
    QgsAbstractRasterLayerLabeling.defaultLabelingForLayer = staticmethod(QgsAbstractRasterLayerLabeling.defaultLabelingForLayer)
    QgsAbstractRasterLayerLabeling.createFromElement = staticmethod(QgsAbstractRasterLayerLabeling.createFromElement)
    QgsAbstractRasterLayerLabeling.__virtual_methods__ = ['multiplyOpacity', 'isInScaleRange', 'toSld', 'accept']
    QgsAbstractRasterLayerLabeling.__abstract_methods__ = ['type', 'clone', 'save', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode']
    QgsAbstractRasterLayerLabeling.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterLayerSimpleLabeling.create = staticmethod(QgsRasterLayerSimpleLabeling.create)
    QgsRasterLayerSimpleLabeling.__overridden_methods__ = ['type', 'clone', 'save', 'accept', 'requiresAdvancedEffects', 'hasNonDefaultCompositionMode', 'multiplyOpacity', 'isInScaleRange']
    QgsRasterLayerSimpleLabeling.__group__ = ['raster']
except (NameError, AttributeError):
    pass
