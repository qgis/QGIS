# The following has been generated automatically from src/core/raster/qgscolorrampshader.h
try:
    QgsColorRampShader.__overridden_methods__ = ['shade', 'legendSymbologyItems']
    import functools as _functools
    __wrapped_QgsColorRampShader_setSourceColorRamp = QgsColorRampShader.setSourceColorRamp
    def __QgsColorRampShader_setSourceColorRamp_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsColorRampShader_setSourceColorRamp(self, arg)
    QgsColorRampShader.setSourceColorRamp = _functools.update_wrapper(__QgsColorRampShader_setSourceColorRamp_wrapper, QgsColorRampShader.setSourceColorRamp)

    import functools as _functools
    __wrapped_QgsColorRampShader_setLegendSettings = QgsColorRampShader.setLegendSettings
    def __QgsColorRampShader_setLegendSettings_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsColorRampShader_setLegendSettings(self, arg)
    QgsColorRampShader.setLegendSettings = _functools.update_wrapper(__QgsColorRampShader_setLegendSettings_wrapper, QgsColorRampShader.setLegendSettings)

    QgsColorRampShader.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsColorRampShader.ColorRampItem.__group__ = ['raster']
except (NameError, AttributeError):
    pass
