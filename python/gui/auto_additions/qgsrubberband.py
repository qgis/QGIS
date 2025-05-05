# The following has been generated automatically from src/gui/qgsrubberband.h
try:
    QgsRubberBand.__overridden_methods__ = ['updatePosition', 'paint']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsRubberBand_setSymbol = QgsRubberBand.setSymbol
    def __QgsRubberBand_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRubberBand_setSymbol(self, arg)
    QgsRubberBand.setSymbol = _functools.update_wrapper(__QgsRubberBand_setSymbol_wrapper, QgsRubberBand.setSymbol)

except (NameError, AttributeError):
    pass
