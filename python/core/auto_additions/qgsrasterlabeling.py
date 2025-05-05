# The following has been generated automatically from src/core/raster/qgsrasterlabeling.h
try:
    QgsAbstractRasterLayerLabeling.defaultLabelingForLayer = staticmethod(QgsAbstractRasterLayerLabeling.defaultLabelingForLayer)
    QgsAbstractRasterLayerLabeling.createFromElement = staticmethod(QgsAbstractRasterLayerLabeling.createFromElement)
    QgsAbstractRasterLayerLabeling.__virtual_methods__ = ['multiplyOpacity', 'isInScaleRange', 'toSld', 'accept']
    QgsAbstractRasterLayerLabeling.__abstract_methods__ = ['type', 'clone', 'save', 'requiresAdvancedEffects']
    QgsAbstractRasterLayerLabeling.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterLayerSimpleLabeling.create = staticmethod(QgsRasterLayerSimpleLabeling.create)
    QgsRasterLayerSimpleLabeling.__overridden_methods__ = ['type', 'clone', 'save', 'accept', 'requiresAdvancedEffects', 'multiplyOpacity', 'isInScaleRange']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterLayerSimpleLabeling_setNumericFormat = QgsRasterLayerSimpleLabeling.setNumericFormat
    def __QgsRasterLayerSimpleLabeling_setNumericFormat_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterLayerSimpleLabeling_setNumericFormat(self, arg)
    QgsRasterLayerSimpleLabeling.setNumericFormat = _functools.update_wrapper(__QgsRasterLayerSimpleLabeling_setNumericFormat_wrapper, QgsRasterLayerSimpleLabeling.setNumericFormat)

    QgsRasterLayerSimpleLabeling.__group__ = ['raster']
except (NameError, AttributeError):
    pass
