# The following has been generated automatically from src/core/./3d/qgsabstract3dsymbol.h
# monkey patching scoped based enum
QgsAbstract3DSymbol.PropertyHeight = QgsAbstract3DSymbol.Property.Height
QgsAbstract3DSymbol.Property.PropertyHeight = QgsAbstract3DSymbol.Property.Height
QgsAbstract3DSymbol.PropertyHeight.is_monkey_patched = True
QgsAbstract3DSymbol.PropertyHeight.__doc__ = "Height (altitude)"
QgsAbstract3DSymbol.PropertyExtrusionHeight = QgsAbstract3DSymbol.Property.ExtrusionHeight
QgsAbstract3DSymbol.Property.PropertyExtrusionHeight = QgsAbstract3DSymbol.Property.ExtrusionHeight
QgsAbstract3DSymbol.PropertyExtrusionHeight.is_monkey_patched = True
QgsAbstract3DSymbol.PropertyExtrusionHeight.__doc__ = "Extrusion height (zero means no extrusion)"
QgsAbstract3DSymbol.Property.__doc__ = """Data definable properties.

* ``PropertyHeight``: Height (altitude)
* ``PropertyExtrusionHeight``: Extrusion height (zero means no extrusion)

"""
# --
try:
    QgsAbstract3DSymbol.__group__ = ['3d']
except NameError:
    pass
