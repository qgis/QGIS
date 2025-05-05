# The following has been generated automatically from src/core/symbology/qgslinesymbol.h
try:
    QgsLineSymbol.createSimple = staticmethod(QgsLineSymbol.createSimple)
    QgsLineSymbol.__overridden_methods__ = ['clone']
    import functools as _functools
    __wrapped_QgsLineSymbol_QgsLineSymbol = QgsLineSymbol.QgsLineSymbol
    def __QgsLineSymbol_QgsLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLineSymbol_QgsLineSymbol(self, arg)
    QgsLineSymbol.QgsLineSymbol = _functools.update_wrapper(__QgsLineSymbol_QgsLineSymbol_wrapper, QgsLineSymbol.QgsLineSymbol)

    QgsLineSymbol.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
