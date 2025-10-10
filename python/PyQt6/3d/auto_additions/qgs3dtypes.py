# The following has been generated automatically from src/3d/qgs3dtypes.h
Qgs3DTypes.NoCulling = Qgs3DTypes.CullingMode.NoCulling
Qgs3DTypes.Front = Qgs3DTypes.CullingMode.Front
Qgs3DTypes.Back = Qgs3DTypes.CullingMode.Back
Qgs3DTypes.FrontAndBack = Qgs3DTypes.CullingMode.FrontAndBack
Qgs3DTypes.Main3DRenderer = Qgs3DTypes.Flag3DRenderer.Main3DRenderer
Qgs3DTypes.Selected3DRenderer = Qgs3DTypes.Flag3DRenderer.Selected3DRenderer
# monkey patching scoped based enum
Qgs3DTypes.ExportFormat.Obj.__doc__ = ""
Qgs3DTypes.ExportFormat.Stl.__doc__ = ""
Qgs3DTypes.ExportFormat.__doc__ = """Scene export format

.. versionadded:: 4.0

* ``Obj``: 
* ``Stl``: 

"""
# --
Qgs3DTypes.ExportFormat.baseClass = Qgs3DTypes
try:
    Qgs3DTypes.__attribute_docs__ = {'PROP_NAME_3D_RENDERER_FLAG': 'Qt property name to hold the 3D geometry renderer flag'}
    Qgs3DTypes.__annotations__ = {'PROP_NAME_3D_RENDERER_FLAG': str}
except (NameError, AttributeError):
    pass
