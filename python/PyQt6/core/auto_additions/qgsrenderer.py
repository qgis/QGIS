# The following has been generated automatically from src/core/symbology/qgsrenderer.h
# monkey patching scoped based enum
QgsFeatureRenderer.Property.HeatmapRadius.__doc__ = "Heatmap renderer radius"
QgsFeatureRenderer.Property.HeatmapMaximum.__doc__ = "Heatmap maximum value"
QgsFeatureRenderer.Property.__doc__ = """Data definable properties for renderers.

.. versionadded:: 3.38

* ``HeatmapRadius``: Heatmap renderer radius
* ``HeatmapMaximum``: Heatmap maximum value

"""
# --
QgsFeatureRenderer.SymbolLevels = QgsFeatureRenderer.Capability.SymbolLevels
QgsFeatureRenderer.MoreSymbolsPerFeature = QgsFeatureRenderer.Capability.MoreSymbolsPerFeature
QgsFeatureRenderer.Filter = QgsFeatureRenderer.Capability.Filter
QgsFeatureRenderer.ScaleDependent = QgsFeatureRenderer.Capability.ScaleDependent
QgsFeatureRenderer.Capabilities = lambda flags=0: QgsFeatureRenderer.Capability(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsFeatureRenderer.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsFeatureRenderer.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFeatureRenderer.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFeatureRenderer.Capability.__or__ = lambda flag1, flag2: QgsFeatureRenderer.Capability(_force_int(flag1) | _force_int(flag2))
try:
    QgsFeatureRenderer.defaultRenderer = staticmethod(QgsFeatureRenderer.defaultRenderer)
    QgsFeatureRenderer.load = staticmethod(QgsFeatureRenderer.load)
    QgsFeatureRenderer.loadSld = staticmethod(QgsFeatureRenderer.loadSld)
    QgsFeatureRenderer._getPoint = staticmethod(QgsFeatureRenderer._getPoint)
    QgsFeatureRenderer.convertSymbolSizeScale = staticmethod(QgsFeatureRenderer.convertSymbolSizeScale)
    QgsFeatureRenderer.convertSymbolRotation = staticmethod(QgsFeatureRenderer.convertSymbolRotation)
    QgsFeatureRenderer.__virtual_methods__ = ['originalSymbolForFeature', 'legendKeysForFeature', 'startRender', 'stopRender', 'canSkipRender', 'filter', 'usesEmbeddedSymbols', 'filterNeedsGeometry', 'renderFeature', 'dump', 'capabilities', 'flags', 'symbols', 'save', 'writeSld', 'toSld', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'setLegendSymbolItem', 'legendKeyToExpression', 'legendSymbolItems', 'createLegendNodes', 'legendClassificationAttribute', 'willRenderFeature', 'symbolsForFeature', 'originalSymbolsForFeature', 'modifyRequestExtent', 'setEmbeddedRenderer', 'accept']
    QgsFeatureRenderer.__abstract_methods__ = ['symbolForFeature', 'usedAttributes', 'clone']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFeatureRenderer_setPaintEffect = QgsFeatureRenderer.setPaintEffect
    def __QgsFeatureRenderer_setPaintEffect_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFeatureRenderer_setPaintEffect(self, arg)
    QgsFeatureRenderer.setPaintEffect = _functools.update_wrapper(__QgsFeatureRenderer_setPaintEffect_wrapper, QgsFeatureRenderer.setPaintEffect)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFeatureRenderer_setEmbeddedRenderer = QgsFeatureRenderer.setEmbeddedRenderer
    def __QgsFeatureRenderer_setEmbeddedRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFeatureRenderer_setEmbeddedRenderer(self, arg)
    QgsFeatureRenderer.setEmbeddedRenderer = _functools.update_wrapper(__QgsFeatureRenderer_setEmbeddedRenderer_wrapper, QgsFeatureRenderer.setEmbeddedRenderer)

    QgsFeatureRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLevelItem.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
