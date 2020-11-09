# The following has been generated automatically from src/3d/materials/qgsabstractmaterialsettings.h
QgsAbstractMaterialSettings.RenderingTechnique = QgsMaterialSettingsRenderingTechnique
# monkey patching scoped based enum
QgsAbstractMaterialSettings.Triangles = QgsMaterialSettingsRenderingTechnique.Triangles
QgsAbstractMaterialSettings.Triangles.__doc__ = "Triangle based rendering (default)"
QgsAbstractMaterialSettings.Lines = QgsMaterialSettingsRenderingTechnique.Lines
QgsAbstractMaterialSettings.Lines.__doc__ = "Line based rendering, requires line data"
QgsAbstractMaterialSettings.InstancedPoints = QgsMaterialSettingsRenderingTechnique.InstancedPoints
QgsAbstractMaterialSettings.InstancedPoints.__doc__ = "Instanced based rendering, requiring triangles and point data"
QgsAbstractMaterialSettings.Points = QgsMaterialSettingsRenderingTechnique.Points
QgsAbstractMaterialSettings.Points.__doc__ = "Point based rendering, requires point data"
QgsAbstractMaterialSettings.TrianglesWithFixedTexture = QgsMaterialSettingsRenderingTechnique.TrianglesWithFixedTexture
QgsAbstractMaterialSettings.TrianglesWithFixedTexture.__doc__ = "Triangle based rendering, using a fixed, non-user-configurable texture (e.g. for terrain rendering)"
QgsMaterialSettingsRenderingTechnique.__doc__ = 'Material rendering techniques\n\n.. versionadded:: 3.16\n\n' + '* ``Triangles``: ' + QgsMaterialSettingsRenderingTechnique.Triangles.__doc__ + '\n' + '* ``Lines``: ' + QgsMaterialSettingsRenderingTechnique.Lines.__doc__ + '\n' + '* ``InstancedPoints``: ' + QgsMaterialSettingsRenderingTechnique.InstancedPoints.__doc__ + '\n' + '* ``Points``: ' + QgsMaterialSettingsRenderingTechnique.Points.__doc__ + '\n' + '* ``TrianglesWithFixedTexture``: ' + QgsMaterialSettingsRenderingTechnique.TrianglesWithFixedTexture.__doc__
# --
