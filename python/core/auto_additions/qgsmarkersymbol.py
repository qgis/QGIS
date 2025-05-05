# The following has been generated automatically from src/core/symbology/qgsmarkersymbol.h
try:
    QgsMarkerSymbol.createSimple = staticmethod(QgsMarkerSymbol.createSimple)
    QgsMarkerSymbol.__overridden_methods__ = ['clone']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMarkerSymbol_QgsMarkerSymbol = QgsMarkerSymbol.QgsMarkerSymbol
    def __QgsMarkerSymbol_QgsMarkerSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMarkerSymbol_QgsMarkerSymbol(self, arg)
    QgsMarkerSymbol.QgsMarkerSymbol = _functools.update_wrapper(__QgsMarkerSymbol_QgsMarkerSymbol_wrapper, QgsMarkerSymbol.QgsMarkerSymbol)

    QgsMarkerSymbol.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
