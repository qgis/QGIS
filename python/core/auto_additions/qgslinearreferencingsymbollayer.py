# The following has been generated automatically from src/core/symbology/qgslinearreferencingsymbollayer.h
try:
    QgsLinearReferencingSymbolLayer.create = staticmethod(QgsLinearReferencingSymbolLayer.create)
    QgsLinearReferencingSymbolLayer.__overridden_methods__ = ['clone', 'properties', 'layerType', 'flags', 'subSymbol', 'setSubSymbol', 'startRender', 'stopRender', 'renderPolyline']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLinearReferencingSymbolLayer_setSubSymbol = QgsLinearReferencingSymbolLayer.setSubSymbol
    def __QgsLinearReferencingSymbolLayer_setSubSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLinearReferencingSymbolLayer_setSubSymbol(self, arg)
    QgsLinearReferencingSymbolLayer.setSubSymbol = _functools.update_wrapper(__QgsLinearReferencingSymbolLayer_setSubSymbol_wrapper, QgsLinearReferencingSymbolLayer.setSubSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLinearReferencingSymbolLayer_setNumericFormat = QgsLinearReferencingSymbolLayer.setNumericFormat
    def __QgsLinearReferencingSymbolLayer_setNumericFormat_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLinearReferencingSymbolLayer_setNumericFormat(self, arg)
    QgsLinearReferencingSymbolLayer.setNumericFormat = _functools.update_wrapper(__QgsLinearReferencingSymbolLayer_setNumericFormat_wrapper, QgsLinearReferencingSymbolLayer.setNumericFormat)

    QgsLinearReferencingSymbolLayer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
