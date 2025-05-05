# The following has been generated automatically from src/core/symbology/qgscategorizedsymbolrenderer.h
try:
    QgsCategorizedSymbolRenderer.create = staticmethod(QgsCategorizedSymbolRenderer.create)
    QgsCategorizedSymbolRenderer.convertFromRenderer = staticmethod(QgsCategorizedSymbolRenderer.convertFromRenderer)
    QgsCategorizedSymbolRenderer.createCategories = staticmethod(QgsCategorizedSymbolRenderer.createCategories)
    QgsCategorizedSymbolRenderer.displayString = staticmethod(QgsCategorizedSymbolRenderer.displayString)
    QgsCategorizedSymbolRenderer.__overridden_methods__ = ['flags', 'symbolForFeature', 'originalSymbolForFeature', 'startRender', 'stopRender', 'usedAttributes', 'filterNeedsGeometry', 'dump', 'clone', 'toSld', 'capabilities', 'filter', 'symbols', 'accept', 'save', 'legendSymbolItems', 'legendKeysForFeature', 'legendKeyToExpression', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'setLegendSymbolItem', 'checkLegendSymbolItem', 'legendClassificationAttribute']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCategorizedSymbolRenderer_setSourceSymbol = QgsCategorizedSymbolRenderer.setSourceSymbol
    def __QgsCategorizedSymbolRenderer_setSourceSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCategorizedSymbolRenderer_setSourceSymbol(self, arg)
    QgsCategorizedSymbolRenderer.setSourceSymbol = _functools.update_wrapper(__QgsCategorizedSymbolRenderer_setSourceSymbol_wrapper, QgsCategorizedSymbolRenderer.setSourceSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCategorizedSymbolRenderer_setSourceColorRamp = QgsCategorizedSymbolRenderer.setSourceColorRamp
    def __QgsCategorizedSymbolRenderer_setSourceColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCategorizedSymbolRenderer_setSourceColorRamp(self, arg)
    QgsCategorizedSymbolRenderer.setSourceColorRamp = _functools.update_wrapper(__QgsCategorizedSymbolRenderer_setSourceColorRamp_wrapper, QgsCategorizedSymbolRenderer.setSourceColorRamp)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCategorizedSymbolRenderer_updateColorRamp = QgsCategorizedSymbolRenderer.updateColorRamp
    def __QgsCategorizedSymbolRenderer_updateColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCategorizedSymbolRenderer_updateColorRamp(self, arg)
    QgsCategorizedSymbolRenderer.updateColorRamp = _functools.update_wrapper(__QgsCategorizedSymbolRenderer_updateColorRamp_wrapper, QgsCategorizedSymbolRenderer.updateColorRamp)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCategorizedSymbolRenderer_setDataDefinedSizeLegend = QgsCategorizedSymbolRenderer.setDataDefinedSizeLegend
    def __QgsCategorizedSymbolRenderer_setDataDefinedSizeLegend_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCategorizedSymbolRenderer_setDataDefinedSizeLegend(self, arg)
    QgsCategorizedSymbolRenderer.setDataDefinedSizeLegend = _functools.update_wrapper(__QgsCategorizedSymbolRenderer_setDataDefinedSizeLegend_wrapper, QgsCategorizedSymbolRenderer.setDataDefinedSizeLegend)

    QgsCategorizedSymbolRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRendererCategory_setSymbol = QgsRendererCategory.setSymbol
    def __QgsRendererCategory_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRendererCategory_setSymbol(self, arg)
    QgsRendererCategory.setSymbol = _functools.update_wrapper(__QgsRendererCategory_setSymbol_wrapper, QgsRendererCategory.setSymbol)

    QgsRendererCategory.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
