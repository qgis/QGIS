# The following has been generated automatically from src/core/symbology/qgsfillsymbol.h
try:
    QgsFillSymbol.createSimple = staticmethod(QgsFillSymbol.createSimple)
    QgsFillSymbol.__overridden_methods__ = ['clone']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFillSymbol_QgsFillSymbol = QgsFillSymbol.QgsFillSymbol
    def __QgsFillSymbol_QgsFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFillSymbol_QgsFillSymbol(self, arg)
    QgsFillSymbol.QgsFillSymbol = _functools.update_wrapper(__QgsFillSymbol_QgsFillSymbol_wrapper, QgsFillSymbol.QgsFillSymbol)

    QgsFillSymbol.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
