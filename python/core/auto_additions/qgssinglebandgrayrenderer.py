# The following has been generated automatically from src/core/raster/qgssinglebandgrayrenderer.h
try:
    QgsSingleBandGrayRenderer.create = staticmethod(QgsSingleBandGrayRenderer.create)
    QgsSingleBandGrayRenderer.__overridden_methods__ = ['clone', 'flags', 'block', 'inputBand', 'setInputBand', 'writeXml', 'legendSymbologyItems', 'createLegendNodes', 'usesBands', 'toSld']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSingleBandGrayRenderer_setContrastEnhancement = QgsSingleBandGrayRenderer.setContrastEnhancement
    def __QgsSingleBandGrayRenderer_setContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleBandGrayRenderer_setContrastEnhancement(self, arg)
    QgsSingleBandGrayRenderer.setContrastEnhancement = _functools.update_wrapper(__QgsSingleBandGrayRenderer_setContrastEnhancement_wrapper, QgsSingleBandGrayRenderer.setContrastEnhancement)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSingleBandGrayRenderer_setLegendSettings = QgsSingleBandGrayRenderer.setLegendSettings
    def __QgsSingleBandGrayRenderer_setLegendSettings_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleBandGrayRenderer_setLegendSettings(self, arg)
    QgsSingleBandGrayRenderer.setLegendSettings = _functools.update_wrapper(__QgsSingleBandGrayRenderer_setLegendSettings_wrapper, QgsSingleBandGrayRenderer.setLegendSettings)

    QgsSingleBandGrayRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
