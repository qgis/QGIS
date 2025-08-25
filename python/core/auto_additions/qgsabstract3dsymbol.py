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
QgsAbstract3DSymbol.ScaleX = QgsAbstract3DSymbol.Property.ScaleX
QgsAbstract3DSymbol.ScaleX.is_monkey_patched = True
QgsAbstract3DSymbol.ScaleX.__doc__ = "X-axis scaling \n.. versionadded:: 4.0"
QgsAbstract3DSymbol.ScaleY = QgsAbstract3DSymbol.Property.ScaleY
QgsAbstract3DSymbol.ScaleY.is_monkey_patched = True
QgsAbstract3DSymbol.ScaleY.__doc__ = "Y-axis scaling \n.. versionadded:: 4.0"
QgsAbstract3DSymbol.ScaleZ = QgsAbstract3DSymbol.Property.ScaleZ
QgsAbstract3DSymbol.ScaleZ.is_monkey_patched = True
QgsAbstract3DSymbol.ScaleZ.__doc__ = "Z-axis scaling \n.. versionadded:: 4.0"
QgsAbstract3DSymbol.Property.__doc__ = """Data definable properties.

* ``Height``: Height (altitude)

  Available as ``QgsAbstract3DSymbol.PropertyHeight`` in older QGIS releases.

* ``ExtrusionHeight``: Extrusion height (zero means no extrusion)

  Available as ``QgsAbstract3DSymbol.PropertyExtrusionHeight`` in older QGIS releases.

* ``ScaleX``: X-axis scaling

  .. versionadded:: 4.0

* ``ScaleY``: Y-axis scaling

  .. versionadded:: 4.0

* ``ScaleZ``: Z-axis scaling

  .. versionadded:: 4.0


"""
# --
try:
    QgsAbstract3DSymbol.__virtual_methods__ = ['compatibleGeometryTypes', 'setDefaultPropertiesFromLayer', 'copyBaseSettings']
    QgsAbstract3DSymbol.__abstract_methods__ = ['type', 'clone', 'writeXml', 'readXml']
    QgsAbstract3DSymbol.__group__ = ['3d']
except (NameError, AttributeError):
    pass
