# The following has been generated automatically from src/core/symbology/qgspointclusterrenderer.h
try:
    QgsPointClusterRenderer.create = staticmethod(QgsPointClusterRenderer.create)
    QgsPointClusterRenderer.convertFromRenderer = staticmethod(QgsPointClusterRenderer.convertFromRenderer)
    QgsPointClusterRenderer.__overridden_methods__ = ['flags', 'clone', 'startRender', 'stopRender', 'save', 'usedAttributes', 'accept', 'drawGroup']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPointClusterRenderer_setClusterSymbol = QgsPointClusterRenderer.setClusterSymbol
    def __QgsPointClusterRenderer_setClusterSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPointClusterRenderer_setClusterSymbol(self, arg)
    QgsPointClusterRenderer.setClusterSymbol = _functools.update_wrapper(__QgsPointClusterRenderer_setClusterSymbol_wrapper, QgsPointClusterRenderer.setClusterSymbol)

    QgsPointClusterRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
