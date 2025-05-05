# The following has been generated automatically from src/gui/qgsrubberband.h
QgsRubberBand.ICON_NONE = QgsRubberBand.IconType.ICON_NONE
QgsRubberBand.ICON_CROSS = QgsRubberBand.IconType.ICON_CROSS
QgsRubberBand.ICON_X = QgsRubberBand.IconType.ICON_X
QgsRubberBand.ICON_BOX = QgsRubberBand.IconType.ICON_BOX
QgsRubberBand.ICON_CIRCLE = QgsRubberBand.IconType.ICON_CIRCLE
QgsRubberBand.ICON_FULL_BOX = QgsRubberBand.IconType.ICON_FULL_BOX
QgsRubberBand.ICON_DIAMOND = QgsRubberBand.IconType.ICON_DIAMOND
QgsRubberBand.ICON_FULL_DIAMOND = QgsRubberBand.IconType.ICON_FULL_DIAMOND
QgsRubberBand.ICON_SVG = QgsRubberBand.IconType.ICON_SVG
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
