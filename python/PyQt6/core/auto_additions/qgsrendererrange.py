# The following has been generated automatically from src/core/symbology/qgsrendererrange.h
try:
    import functools as _functools
    __wrapped_QgsRendererRange_setSymbol = QgsRendererRange.setSymbol
    def __QgsRendererRange_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRendererRange_setSymbol(self, arg)
    QgsRendererRange.setSymbol = _functools.update_wrapper(__QgsRendererRange_setSymbol_wrapper, QgsRendererRange.setSymbol)

    QgsRendererRange.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRendererRangeLabelFormat.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
