# The following has been generated automatically from src/core/symbology/qgssinglesymbolrenderer.h
try:
    QgsSingleSymbolRenderer.createFromSld = staticmethod(QgsSingleSymbolRenderer.createFromSld)
    QgsSingleSymbolRenderer.create = staticmethod(QgsSingleSymbolRenderer.create)
    QgsSingleSymbolRenderer.convertFromRenderer = staticmethod(QgsSingleSymbolRenderer.convertFromRenderer)
    QgsSingleSymbolRenderer.__overridden_methods__ = ['flags', 'symbolForFeature', 'originalSymbolForFeature', 'startRender', 'stopRender', 'usedAttributes', 'accept', 'dump', 'clone', 'toSld', 'capabilities', 'symbols', 'save', 'legendSymbolItems', 'legendKeysForFeature', 'legendKeyToExpression', 'setLegendSymbolItem']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSingleSymbolRenderer_QgsSingleSymbolRenderer = QgsSingleSymbolRenderer.QgsSingleSymbolRenderer
    def __QgsSingleSymbolRenderer_QgsSingleSymbolRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleSymbolRenderer_QgsSingleSymbolRenderer(self, arg)
    QgsSingleSymbolRenderer.QgsSingleSymbolRenderer = _functools.update_wrapper(__QgsSingleSymbolRenderer_QgsSingleSymbolRenderer_wrapper, QgsSingleSymbolRenderer.QgsSingleSymbolRenderer)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSingleSymbolRenderer_setSymbol = QgsSingleSymbolRenderer.setSymbol
    def __QgsSingleSymbolRenderer_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleSymbolRenderer_setSymbol(self, arg)
    QgsSingleSymbolRenderer.setSymbol = _functools.update_wrapper(__QgsSingleSymbolRenderer_setSymbol_wrapper, QgsSingleSymbolRenderer.setSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSingleSymbolRenderer_setDataDefinedSizeLegend = QgsSingleSymbolRenderer.setDataDefinedSizeLegend
    def __QgsSingleSymbolRenderer_setDataDefinedSizeLegend_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleSymbolRenderer_setDataDefinedSizeLegend(self, arg)
    QgsSingleSymbolRenderer.setDataDefinedSizeLegend = _functools.update_wrapper(__QgsSingleSymbolRenderer_setDataDefinedSizeLegend_wrapper, QgsSingleSymbolRenderer.setDataDefinedSizeLegend)

    QgsSingleSymbolRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
