# The following has been generated automatically from src/core/symbology/qgspointdisplacementrenderer.h
QgsPointDisplacementRenderer.Ring = QgsPointDisplacementRenderer.Placement.Ring
QgsPointDisplacementRenderer.ConcentricRings = QgsPointDisplacementRenderer.Placement.ConcentricRings
QgsPointDisplacementRenderer.Grid = QgsPointDisplacementRenderer.Placement.Grid
try:
    QgsPointDisplacementRenderer.create = staticmethod(QgsPointDisplacementRenderer.create)
    QgsPointDisplacementRenderer.convertFromRenderer = staticmethod(QgsPointDisplacementRenderer.convertFromRenderer)
    QgsPointDisplacementRenderer.__overridden_methods__ = ['flags', 'clone', 'startRender', 'stopRender', 'save', 'usedAttributes', 'accept', 'drawGroup']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointDisplacementRenderer_setCenterSymbol = QgsPointDisplacementRenderer.setCenterSymbol
    def __QgsPointDisplacementRenderer_setCenterSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointDisplacementRenderer_setCenterSymbol(self, arg)
    QgsPointDisplacementRenderer.setCenterSymbol = _functools.update_wrapper(__QgsPointDisplacementRenderer_setCenterSymbol_wrapper, QgsPointDisplacementRenderer.setCenterSymbol)

    QgsPointDisplacementRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
