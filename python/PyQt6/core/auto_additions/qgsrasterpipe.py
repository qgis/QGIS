# The following has been generated automatically from src/core/raster/qgsrasterpipe.h
# monkey patching scoped based enum
QgsRasterPipe.RendererOpacity = QgsRasterPipe.Property.RendererOpacity
QgsRasterPipe.RendererOpacity.is_monkey_patched = True
QgsRasterPipe.RendererOpacity.__doc__ = "Raster renderer global opacity"
QgsRasterPipe.Property.__doc__ = "Data definable properties.\n\n.. versionadded:: 3.22\n\n" + '* ``RendererOpacity``: ' + QgsRasterPipe.Property.RendererOpacity.__doc__
# --
QgsRasterPipe.propertyDefinitions = staticmethod(QgsRasterPipe.propertyDefinitions)
