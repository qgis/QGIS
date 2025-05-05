# The following has been generated automatically from src/core/raster/qgsrastercontourrenderer.h
try:
    QgsRasterContourRenderer.create = staticmethod(QgsRasterContourRenderer.create)
    QgsRasterContourRenderer.__overridden_methods__ = ['clone', 'flags', 'writeXml', 'block', 'usesBands', 'createLegendNodes', 'inputBand', 'setInputBand']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterContourRenderer_setContourSymbol = QgsRasterContourRenderer.setContourSymbol
    def __QgsRasterContourRenderer_setContourSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterContourRenderer_setContourSymbol(self, arg)
    QgsRasterContourRenderer.setContourSymbol = _functools.update_wrapper(__QgsRasterContourRenderer_setContourSymbol_wrapper, QgsRasterContourRenderer.setContourSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRasterContourRenderer_setContourIndexSymbol = QgsRasterContourRenderer.setContourIndexSymbol
    def __QgsRasterContourRenderer_setContourIndexSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterContourRenderer_setContourIndexSymbol(self, arg)
    QgsRasterContourRenderer.setContourIndexSymbol = _functools.update_wrapper(__QgsRasterContourRenderer_setContourIndexSymbol_wrapper, QgsRasterContourRenderer.setContourIndexSymbol)

    QgsRasterContourRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
