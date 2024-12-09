# The following has been generated automatically from src/core/raster/qgsrasterpipe.h
# monkey patching scoped based enum
QgsRasterPipe.RendererOpacity = QgsRasterPipe.Property.RendererOpacity
QgsRasterPipe.RendererOpacity.is_monkey_patched = True
QgsRasterPipe.RendererOpacity.__doc__ = "Raster renderer global opacity"
QgsRasterPipe.Property.__doc__ = """Data definable properties.

.. versionadded:: 3.22

* ``RendererOpacity``: Raster renderer global opacity

"""
# --
try:
    QgsRasterPipe.propertyDefinitions = staticmethod(QgsRasterPipe.propertyDefinitions)
    QgsRasterPipe.__group__ = ['raster']
except (NameError, AttributeError):
    pass
