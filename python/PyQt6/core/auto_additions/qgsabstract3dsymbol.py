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
QgsAbstract3DSymbol.ScaleX.__doc__ = "X-axis scaling \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.ScaleY = QgsAbstract3DSymbol.Property.ScaleY
QgsAbstract3DSymbol.ScaleY.is_monkey_patched = True
QgsAbstract3DSymbol.ScaleY.__doc__ = "Y-axis scaling \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.ScaleZ = QgsAbstract3DSymbol.Property.ScaleZ
QgsAbstract3DSymbol.ScaleZ.is_monkey_patched = True
QgsAbstract3DSymbol.ScaleZ.__doc__ = "Z-axis scaling \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.TranslationX = QgsAbstract3DSymbol.Property.TranslationX
QgsAbstract3DSymbol.TranslationX.is_monkey_patched = True
QgsAbstract3DSymbol.TranslationX.__doc__ = "X-axis translation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.TranslationY = QgsAbstract3DSymbol.Property.TranslationY
QgsAbstract3DSymbol.TranslationY.is_monkey_patched = True
QgsAbstract3DSymbol.TranslationY.__doc__ = "Y-axis translation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.TranslationZ = QgsAbstract3DSymbol.Property.TranslationZ
QgsAbstract3DSymbol.TranslationZ.is_monkey_patched = True
QgsAbstract3DSymbol.TranslationZ.__doc__ = "Z-axis translation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.RotationX = QgsAbstract3DSymbol.Property.RotationX
QgsAbstract3DSymbol.RotationX.is_monkey_patched = True
QgsAbstract3DSymbol.RotationX.__doc__ = "X-axis rotation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.RotationY = QgsAbstract3DSymbol.Property.RotationY
QgsAbstract3DSymbol.RotationY.is_monkey_patched = True
QgsAbstract3DSymbol.RotationY.__doc__ = "Y-axis rotation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.RotationZ = QgsAbstract3DSymbol.Property.RotationZ
QgsAbstract3DSymbol.RotationZ.is_monkey_patched = True
QgsAbstract3DSymbol.RotationZ.__doc__ = "Z-axis rotation \n.. versionadded:: 4.2"
QgsAbstract3DSymbol.Property.__doc__ = """Data definable properties.

* ``Height``: Height (altitude)

  Available as ``QgsAbstract3DSymbol.PropertyHeight`` in older QGIS releases.

* ``ExtrusionHeight``: Extrusion height (zero means no extrusion)

  Available as ``QgsAbstract3DSymbol.PropertyExtrusionHeight`` in older QGIS releases.

* ``ScaleX``: X-axis scaling

  .. versionadded:: 4.2

* ``ScaleY``: Y-axis scaling

  .. versionadded:: 4.2

* ``ScaleZ``: Z-axis scaling

  .. versionadded:: 4.2

* ``TranslationX``: X-axis translation

  .. versionadded:: 4.2

* ``TranslationY``: Y-axis translation

  .. versionadded:: 4.2

* ``TranslationZ``: Z-axis translation

  .. versionadded:: 4.2

* ``RotationX``: X-axis rotation

  .. versionadded:: 4.2

* ``RotationY``: Y-axis rotation

  .. versionadded:: 4.2

* ``RotationZ``: Z-axis rotation

  .. versionadded:: 4.2


"""
# --
try:
    QgsAbstract3DSymbol.__virtual_methods__ = ['compatibleGeometryTypes', 'setDefaultPropertiesFromLayer', 'copyBaseSettings']
    QgsAbstract3DSymbol.__abstract_methods__ = ['type', 'clone', 'writeXml', 'readXml', 'setMaterialSettings']
    QgsAbstract3DSymbol.__group__ = ['3d']
except (NameError, AttributeError):
    pass
