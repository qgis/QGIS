# The following has been generated automatically from src/core/symbology/qgssymbol.h
# monkey patching scoped based enum
QgsSymbol.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.Property.PropertyOpacity = QgsSymbol.Property.Opacity
QgsSymbol.PropertyOpacity.is_monkey_patched = True
QgsSymbol.PropertyOpacity.__doc__ = "Opacity"
QgsSymbol.Property.__doc__ = "Data definable properties.\n\n.. versionadded:: 3.18\n\n" + '* ``PropertyOpacity``: ' + QgsSymbol.Property.Opacity.__doc__
# --
QgsSymbol.symbolTypeToString = staticmethod(QgsSymbol.symbolTypeToString)
QgsSymbol.symbolTypeForGeometryType = staticmethod(QgsSymbol.symbolTypeForGeometryType)
QgsSymbol.defaultSymbol = staticmethod(QgsSymbol.defaultSymbol)
QgsSymbol._getPoint = staticmethod(QgsSymbol._getPoint)
QgsSymbol._getLineString = staticmethod(QgsSymbol._getLineString)
QgsSymbol._getPolygonRing = staticmethod(QgsSymbol._getPolygonRing)
QgsSymbol._getPolygon = staticmethod(QgsSymbol._getPolygon)
