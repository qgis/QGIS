# The following has been generated automatically from src/3d/materials/qgsabstractmaterialsettings.h
QgsAbstractMaterialSettings.RenderingTechnique = QgsMaterialSettingsRenderingTechnique
# monkey patching scoped based enum
QgsAbstractMaterialSettings.Triangles = QgsMaterialSettingsRenderingTechnique.Triangles
QgsAbstractMaterialSettings.Triangles.is_monkey_patched = True
QgsAbstractMaterialSettings.Triangles.__doc__ = "Triangle based rendering (default)"
QgsAbstractMaterialSettings.Lines = QgsMaterialSettingsRenderingTechnique.Lines
QgsAbstractMaterialSettings.Lines.is_monkey_patched = True
QgsAbstractMaterialSettings.Lines.__doc__ = "Line based rendering, requires line data"
QgsAbstractMaterialSettings.InstancedPoints = QgsMaterialSettingsRenderingTechnique.InstancedPoints
QgsAbstractMaterialSettings.InstancedPoints.is_monkey_patched = True
QgsAbstractMaterialSettings.InstancedPoints.__doc__ = "Instanced based rendering, requiring triangles and point data"
QgsAbstractMaterialSettings.Points = QgsMaterialSettingsRenderingTechnique.Points
QgsAbstractMaterialSettings.Points.is_monkey_patched = True
QgsAbstractMaterialSettings.Points.__doc__ = "Point based rendering, requires point data"
QgsAbstractMaterialSettings.TrianglesWithFixedTexture = QgsMaterialSettingsRenderingTechnique.TrianglesWithFixedTexture
QgsAbstractMaterialSettings.TrianglesWithFixedTexture.is_monkey_patched = True
QgsAbstractMaterialSettings.TrianglesWithFixedTexture.__doc__ = "Triangle based rendering, using a fixed, non-user-configurable texture (e.g. for terrain rendering)"
QgsAbstractMaterialSettings.TrianglesFromModel = QgsMaterialSettingsRenderingTechnique.TrianglesFromModel
QgsAbstractMaterialSettings.TrianglesFromModel.is_monkey_patched = True
QgsAbstractMaterialSettings.TrianglesFromModel.__doc__ = "Triangle based rendering, using a model object source"
QgsAbstractMaterialSettings.TrianglesDataDefined = QgsMaterialSettingsRenderingTechnique.TrianglesDataDefined
QgsAbstractMaterialSettings.TrianglesDataDefined.is_monkey_patched = True
QgsAbstractMaterialSettings.TrianglesDataDefined.__doc__ = "Triangle based rendering with possibility of datadefined color \n.. versionadded:: 3.18"
QgsMaterialSettingsRenderingTechnique.__doc__ = """Material rendering techniques

.. versionadded:: 3.16

* ``Triangles``: Triangle based rendering (default)
* ``Lines``: Line based rendering, requires line data
* ``InstancedPoints``: Instanced based rendering, requiring triangles and point data
* ``Points``: Point based rendering, requires point data
* ``TrianglesWithFixedTexture``: Triangle based rendering, using a fixed, non-user-configurable texture (e.g. for terrain rendering)
* ``TrianglesFromModel``: Triangle based rendering, using a model object source
* ``TrianglesDataDefined``: Triangle based rendering with possibility of datadefined color

  .. versionadded:: 3.18


"""
# --
try:
    QgsMaterialContext.__group__ = ['materials']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractMaterialSettings.__group__ = ['materials']
except (NameError, AttributeError):
    pass
