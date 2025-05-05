# The following has been generated automatically from src/core/raster/qgsrastershader.h
try:
    import functools as _functools
    __wrapped_QgsRasterShader_setRasterShaderFunction = QgsRasterShader.setRasterShaderFunction
    def __QgsRasterShader_setRasterShaderFunction_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterShader_setRasterShaderFunction(self, arg)
    QgsRasterShader.setRasterShaderFunction = _functools.update_wrapper(__QgsRasterShader_setRasterShaderFunction_wrapper, QgsRasterShader.setRasterShaderFunction)

    QgsRasterShader.__group__ = ['raster']
except (NameError, AttributeError):
    pass
