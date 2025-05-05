# The following has been generated automatically from src/core/symbology/qgsmergedfeaturerenderer.h
QgsMergedFeatureRenderer.Merge = QgsMergedFeatureRenderer.GeometryOperation.Merge
QgsMergedFeatureRenderer.InvertOnly = QgsMergedFeatureRenderer.GeometryOperation.InvertOnly
QgsMergedFeatureRenderer.MergeAndInvert = QgsMergedFeatureRenderer.GeometryOperation.MergeAndInvert
try:
    QgsMergedFeatureRenderer.create = staticmethod(QgsMergedFeatureRenderer.create)
    QgsMergedFeatureRenderer.convertFromRenderer = staticmethod(QgsMergedFeatureRenderer.convertFromRenderer)
    QgsMergedFeatureRenderer.__overridden_methods__ = ['clone', 'startRender', 'flags', 'renderFeature', 'stopRender', 'dump', 'usedAttributes', 'filterNeedsGeometry', 'capabilities', 'symbols', 'symbolForFeature', 'originalSymbolForFeature', 'symbolsForFeature', 'originalSymbolsForFeature', 'legendKeysForFeature', 'legendKeyToExpression', 'legendSymbolItems', 'willRenderFeature', 'save', 'setEmbeddedRenderer', 'embeddedRenderer', 'setLegendSymbolItem', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'accept']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMergedFeatureRenderer_QgsMergedFeatureRenderer = QgsMergedFeatureRenderer.QgsMergedFeatureRenderer
    def __QgsMergedFeatureRenderer_QgsMergedFeatureRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMergedFeatureRenderer_QgsMergedFeatureRenderer(self, arg)
    QgsMergedFeatureRenderer.QgsMergedFeatureRenderer = _functools.update_wrapper(__QgsMergedFeatureRenderer_QgsMergedFeatureRenderer_wrapper, QgsMergedFeatureRenderer.QgsMergedFeatureRenderer)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMergedFeatureRenderer_embeddedRenderer = QgsMergedFeatureRenderer.embeddedRenderer
    def __QgsMergedFeatureRenderer_embeddedRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMergedFeatureRenderer_embeddedRenderer(self, arg)
    QgsMergedFeatureRenderer.embeddedRenderer = _functools.update_wrapper(__QgsMergedFeatureRenderer_embeddedRenderer_wrapper, QgsMergedFeatureRenderer.embeddedRenderer)

    QgsMergedFeatureRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
