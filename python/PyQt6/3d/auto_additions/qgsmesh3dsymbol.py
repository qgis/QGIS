# The following has been generated automatically from src/3d/symbols/qgsmesh3dsymbol.h
# monkey patching scoped based enum
QgsMesh3DSymbol.RenderingStyle.SingleColor.__doc__ = "Render the mesh with a single color"
QgsMesh3DSymbol.RenderingStyle.ColorRamp.__doc__ = "Render the mesh with a color ramp"
QgsMesh3DSymbol.RenderingStyle.ColorRamp2DRendering.__doc__ = "Render the mesh with the color ramp shader of the 2D rendering"
QgsMesh3DSymbol.RenderingStyle.__doc__ = """How to render the color of the mesh

.. versionadded:: 3.12

* ``SingleColor``: Render the mesh with a single color
* ``ColorRamp``: Render the mesh with a color ramp
* ``ColorRamp2DRendering``: Render the mesh with the color ramp shader of the 2D rendering

"""
# --
# monkey patching scoped based enum
QgsMesh3DSymbol.ZValueType.VerticesZValue.__doc__ = "Use the Z value of the vertices"
QgsMesh3DSymbol.ZValueType.ScalarDatasetZvalue.__doc__ = "Use the value from a dataset (for example, water surface value)"
QgsMesh3DSymbol.ZValueType.__doc__ = """How to render the Z value of the mesh

.. versionadded:: 3.14

* ``VerticesZValue``: Use the Z value of the vertices
* ``ScalarDatasetZvalue``: Use the value from a dataset (for example, water surface value)

"""
# --
try:
    QgsMesh3DSymbol.__group__ = ['symbols']
except (NameError, AttributeError):
    pass
