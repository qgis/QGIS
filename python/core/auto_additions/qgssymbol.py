# The following has been generated automatically from src/core/symbology/qgssymbol.h
# monkey patching scoped based enum
QgsSymbol.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.Property.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.PropertyOpacity.is_monkey_patched = True
QgsSymbol.PropertyOpacity.__doc__ = "Opacity"
QgsSymbol.ExtentBuffer = QgsSymbol.Property.ExtentBuffer
QgsSymbol.ExtentBuffer.is_monkey_patched = True
QgsSymbol.ExtentBuffer.__doc__ = "Extent buffer \n.. versionadded:: 3.42"
QgsSymbol.Property.__doc__ = """Data definable properties.

.. versionadded:: 3.18

* ``Opacity``: Opacity

  Available as ``QgsSymbol.PropertyOpacity`` in older QGIS releases.

* ``ExtentBuffer``: Extent buffer

  .. versionadded:: 3.42


"""
# --
try:
    QgsSymbol.symbolTypeToString = staticmethod(QgsSymbol.symbolTypeToString)
    QgsSymbol.symbolTypeForGeometryType = staticmethod(QgsSymbol.symbolTypeForGeometryType)
    QgsSymbol.defaultSymbol = staticmethod(QgsSymbol.defaultSymbol)
    QgsSymbol._getPoint = staticmethod(QgsSymbol._getPoint)
    QgsSymbol._getLineString = staticmethod(QgsSymbol._getLineString)
    QgsSymbol._getPolygonRing = staticmethod(QgsSymbol._getPolygonRing)
    QgsSymbol._getPolygon = staticmethod(QgsSymbol._getPolygon)
    QgsSymbol.__abstract_methods__ = ['clone']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSymbol_appendSymbolLayer = QgsSymbol.appendSymbolLayer
    def __QgsSymbol_appendSymbolLayer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbol_appendSymbolLayer(self, arg)
    QgsSymbol.appendSymbolLayer = _functools.update_wrapper(__QgsSymbol_appendSymbolLayer_wrapper, QgsSymbol.appendSymbolLayer)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSymbol_setBufferSettings = QgsSymbol.setBufferSettings
    def __QgsSymbol_setBufferSettings_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbol_setBufferSettings(self, arg)
    QgsSymbol.setBufferSettings = _functools.update_wrapper(__QgsSymbol_setBufferSettings_wrapper, QgsSymbol.setBufferSettings)

    QgsSymbol.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSymbolBufferSettings_setFillSymbol = QgsSymbolBufferSettings.setFillSymbol
    def __QgsSymbolBufferSettings_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSymbolBufferSettings_setFillSymbol(self, arg)
    QgsSymbolBufferSettings.setFillSymbol = _functools.update_wrapper(__QgsSymbolBufferSettings_setFillSymbol_wrapper, QgsSymbolBufferSettings.setFillSymbol)

    QgsSymbolBufferSettings.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolAnimationSettings.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
