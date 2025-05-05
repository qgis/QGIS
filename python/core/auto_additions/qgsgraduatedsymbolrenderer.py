# The following has been generated automatically from src/core/symbology/qgsgraduatedsymbolrenderer.h
try:
    QgsGraduatedSymbolRenderer.makeBreaksSymmetric = staticmethod(QgsGraduatedSymbolRenderer.makeBreaksSymmetric)
    QgsGraduatedSymbolRenderer.calcEqualIntervalBreaks = staticmethod(QgsGraduatedSymbolRenderer.calcEqualIntervalBreaks)
    QgsGraduatedSymbolRenderer.createRenderer = staticmethod(QgsGraduatedSymbolRenderer.createRenderer)
    QgsGraduatedSymbolRenderer.create = staticmethod(QgsGraduatedSymbolRenderer.create)
    QgsGraduatedSymbolRenderer.convertFromRenderer = staticmethod(QgsGraduatedSymbolRenderer.convertFromRenderer)
    QgsGraduatedSymbolRenderer.__overridden_methods__ = ['flags', 'symbolForFeature', 'originalSymbolForFeature', 'startRender', 'stopRender', 'usedAttributes', 'filterNeedsGeometry', 'dump', 'clone', 'toSld', 'capabilities', 'symbols', 'accept', 'save', 'legendSymbolItems', 'legendKeysForFeature', 'legendKeyToExpression', 'legendSymbolItemsCheckable', 'legendSymbolItemChecked', 'checkLegendSymbolItem', 'setLegendSymbolItem', 'legendClassificationAttribute']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGraduatedSymbolRenderer_setClassificationMethod = QgsGraduatedSymbolRenderer.setClassificationMethod
    def __QgsGraduatedSymbolRenderer_setClassificationMethod_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraduatedSymbolRenderer_setClassificationMethod(self, arg)
    QgsGraduatedSymbolRenderer.setClassificationMethod = _functools.update_wrapper(__QgsGraduatedSymbolRenderer_setClassificationMethod_wrapper, QgsGraduatedSymbolRenderer.setClassificationMethod)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGraduatedSymbolRenderer_setSourceSymbol = QgsGraduatedSymbolRenderer.setSourceSymbol
    def __QgsGraduatedSymbolRenderer_setSourceSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraduatedSymbolRenderer_setSourceSymbol(self, arg)
    QgsGraduatedSymbolRenderer.setSourceSymbol = _functools.update_wrapper(__QgsGraduatedSymbolRenderer_setSourceSymbol_wrapper, QgsGraduatedSymbolRenderer.setSourceSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGraduatedSymbolRenderer_setSourceColorRamp = QgsGraduatedSymbolRenderer.setSourceColorRamp
    def __QgsGraduatedSymbolRenderer_setSourceColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraduatedSymbolRenderer_setSourceColorRamp(self, arg)
    QgsGraduatedSymbolRenderer.setSourceColorRamp = _functools.update_wrapper(__QgsGraduatedSymbolRenderer_setSourceColorRamp_wrapper, QgsGraduatedSymbolRenderer.setSourceColorRamp)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGraduatedSymbolRenderer_updateColorRamp = QgsGraduatedSymbolRenderer.updateColorRamp
    def __QgsGraduatedSymbolRenderer_updateColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraduatedSymbolRenderer_updateColorRamp(self, arg)
    QgsGraduatedSymbolRenderer.updateColorRamp = _functools.update_wrapper(__QgsGraduatedSymbolRenderer_updateColorRamp_wrapper, QgsGraduatedSymbolRenderer.updateColorRamp)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGraduatedSymbolRenderer_setDataDefinedSizeLegend = QgsGraduatedSymbolRenderer.setDataDefinedSizeLegend
    def __QgsGraduatedSymbolRenderer_setDataDefinedSizeLegend_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGraduatedSymbolRenderer_setDataDefinedSizeLegend(self, arg)
    QgsGraduatedSymbolRenderer.setDataDefinedSizeLegend = _functools.update_wrapper(__QgsGraduatedSymbolRenderer_setDataDefinedSizeLegend_wrapper, QgsGraduatedSymbolRenderer.setDataDefinedSizeLegend)

    QgsGraduatedSymbolRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
