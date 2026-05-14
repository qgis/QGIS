# The following has been generated automatically from src/core/./3d/materials/qgsabstractmaterialsettings.h
# monkey patching scoped based enum
QgsAbstractMaterialSettings.Diffuse = QgsAbstractMaterialSettings.Property.Diffuse
QgsAbstractMaterialSettings.Diffuse.is_monkey_patched = True
QgsAbstractMaterialSettings.Diffuse.__doc__ = "Diffuse color"
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
QgsAbstractMaterialSettings.Property.__doc__ = """Data definable properties.

* ``Diffuse``: Diffuse color
* ``Ambient``: Ambient color (phong material)
* ``Warm``: Warm color (gooch material)
* ``Cool``: Cool color (gooch material)
* ``Specular``: Specular color

"""
# --
try:
    QgsAbstractMaterialSettings.__virtual_methods__ = ['readXml', 'writeXml', 'requiresTextureCoordinates', 'requiresTangents']
    QgsAbstractMaterialSettings.__abstract_methods__ = ['type', 'clone', 'equals', 'averageColor', 'setColorsFromBase']
    QgsAbstractMaterialSettings.__group__ = ['3d', 'materials']
except (NameError, AttributeError):
    pass
