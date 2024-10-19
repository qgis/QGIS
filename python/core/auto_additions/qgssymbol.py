# The following has been generated automatically from src/core/symbology/qgssymbol.h
# monkey patching scoped based enum
QgsSymbol.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.Property.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.PropertyOpacity.is_monkey_patched = True
QgsSymbol.PropertyOpacity.__doc__ = "Opacity"
QgsSymbol.Property.__doc__ = """Data definable properties.

.. versionadded:: 3.18

* ``Opacity``: Opacity

  Available as ``QgsSymbol.PropertyOpacity`` in older QGIS releases.


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
    QgsSymbol.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsSymbolAnimationSettings.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsSymbolBufferSettings.__group__ = ['symbology']
except NameError:
    pass
