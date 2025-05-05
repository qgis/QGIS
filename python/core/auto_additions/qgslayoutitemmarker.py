# The following has been generated automatically from src/core/layout/qgslayoutitemmarker.h
try:
    QgsLayoutItemMarker.create = staticmethod(QgsLayoutItemMarker.create)
    QgsLayoutItemMarker.__overridden_methods__ = ['type', 'icon', 'boundingRect', 'fixedSize', 'accept', 'draw', 'writePropertiesToElement', 'readPropertiesFromElement', 'finalizeRestoreFromXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemMarker_setSymbol = QgsLayoutItemMarker.setSymbol
    def __QgsLayoutItemMarker_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMarker_setSymbol(self, arg)
    QgsLayoutItemMarker.setSymbol = _functools.update_wrapper(__QgsLayoutItemMarker_setSymbol_wrapper, QgsLayoutItemMarker.setSymbol)

    QgsLayoutItemMarker.__group__ = ['layout']
except (NameError, AttributeError):
    pass
