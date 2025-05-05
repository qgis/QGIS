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
    import functools as _functools
    __wrapped_QgsRasterPipe_set = QgsRasterPipe.set
    def __QgsRasterPipe_set_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterPipe_set(self, arg)
    QgsRasterPipe.set = _functools.update_wrapper(__QgsRasterPipe_set_wrapper, QgsRasterPipe.set)

    QgsRasterPipe.__group__ = ['raster']
except (NameError, AttributeError):
    pass
