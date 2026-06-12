# The following has been generated automatically from src/core/./3d/materials/qgsabstractmaterialsettings.h
# monkey patching scoped based enum
QgsAbstractMaterialSettings.Diffuse = QgsAbstractMaterialSettings.Property.Diffuse
QgsAbstractMaterialSettings.Diffuse.is_monkey_patched = True
QgsAbstractMaterialSettings.Diffuse.__doc__ = "Diffuse color (phong material)"
QgsAbstractMaterialSettings.Ambient = QgsAbstractMaterialSettings.Property.Ambient
QgsAbstractMaterialSettings.Ambient.is_monkey_patched = True
QgsAbstractMaterialSettings.Ambient.__doc__ = "Ambient color (phong material)"
QgsAbstractMaterialSettings.Warm = QgsAbstractMaterialSettings.Property.Warm
QgsAbstractMaterialSettings.Warm.is_monkey_patched = True
QgsAbstractMaterialSettings.Warm.__doc__ = "Warm color (gooch material)"
QgsAbstractMaterialSettings.Cool = QgsAbstractMaterialSettings.Property.Cool
QgsAbstractMaterialSettings.Cool.is_monkey_patched = True
QgsAbstractMaterialSettings.Cool.__doc__ = "Cool color (gooch material)"
QgsAbstractMaterialSettings.Specular = QgsAbstractMaterialSettings.Property.Specular
QgsAbstractMaterialSettings.Specular.is_monkey_patched = True
QgsAbstractMaterialSettings.Specular.__doc__ = "Specular color"
QgsAbstractMaterialSettings.BaseColor = QgsAbstractMaterialSettings.Property.BaseColor
QgsAbstractMaterialSettings.BaseColor.is_monkey_patched = True
QgsAbstractMaterialSettings.BaseColor.__doc__ = "Base color (metal-rough material) \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.EmissionColor = QgsAbstractMaterialSettings.Property.EmissionColor
QgsAbstractMaterialSettings.EmissionColor.is_monkey_patched = True
QgsAbstractMaterialSettings.EmissionColor.__doc__ = "Emission color (metal-rough material) \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.TextureScale = QgsAbstractMaterialSettings.Property.TextureScale
QgsAbstractMaterialSettings.TextureScale.is_monkey_patched = True
QgsAbstractMaterialSettings.TextureScale.__doc__ = "Texture scale \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.TextureRotation = QgsAbstractMaterialSettings.Property.TextureRotation
QgsAbstractMaterialSettings.TextureRotation.is_monkey_patched = True
QgsAbstractMaterialSettings.TextureRotation.__doc__ = "Texture rotation \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.TextureOffset = QgsAbstractMaterialSettings.Property.TextureOffset
QgsAbstractMaterialSettings.TextureOffset.is_monkey_patched = True
QgsAbstractMaterialSettings.TextureOffset.__doc__ = "Texture offset \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.SheenColor = QgsAbstractMaterialSettings.Property.SheenColor
QgsAbstractMaterialSettings.SheenColor.is_monkey_patched = True
QgsAbstractMaterialSettings.SheenColor.__doc__ = "Sheen color (cloth material) \n.. versionadded:: 4.2"
QgsAbstractMaterialSettings.Property.__doc__ = """Data definable properties.

* ``Diffuse``: Diffuse color (phong material)
* ``Ambient``: Ambient color (phong material)
* ``Warm``: Warm color (gooch material)
* ``Cool``: Cool color (gooch material)
* ``Specular``: Specular color
* ``BaseColor``: Base color (metal-rough material)

  .. versionadded:: 4.2

* ``EmissionColor``: Emission color (metal-rough material)

  .. versionadded:: 4.2

* ``TextureScale``: Texture scale

  .. versionadded:: 4.2

* ``TextureRotation``: Texture rotation

  .. versionadded:: 4.2

* ``TextureOffset``: Texture offset

  .. versionadded:: 4.2

* ``SheenColor``: Sheen color (cloth material)

  .. versionadded:: 4.2


"""
# --
try:
    QgsAbstractMaterialSettings.__virtual_methods__ = ['readXml', 'writeXml', 'requiresTextureCoordinates', 'requiresTangents', 'supportedProperties']
    QgsAbstractMaterialSettings.__abstract_methods__ = ['type', 'clone', 'equals', 'averageColor', 'setColorsFromBase']
    QgsAbstractMaterialSettings.__group__ = ['3d', 'materials']
except (NameError, AttributeError):
    pass
