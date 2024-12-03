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

* ``Height``: Height (altitude)

  Available as ``QgsAbstract3DSymbol.PropertyHeight`` in older QGIS releases.

* ``ExtrusionHeight``: Extrusion height (zero means no extrusion)

  Available as ``QgsAbstract3DSymbol.PropertyExtrusionHeight`` in older QGIS releases.


"""
# --
try:
    QgsAbstract3DSymbol.__group__ = ['3d']
except (NameError, AttributeError):
    pass
