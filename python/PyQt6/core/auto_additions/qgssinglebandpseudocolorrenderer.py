# The following has been generated automatically from src/core/raster/qgssinglebandpseudocolorrenderer.h
try:
    QgsSingleBandPseudoColorRenderer.create = staticmethod(QgsSingleBandPseudoColorRenderer.create)
    QgsSingleBandPseudoColorRenderer.__overridden_methods__ = ['clone', 'flags', 'block', 'canCreateRasterAttributeTable', 'writeXml', 'legendSymbologyItems', 'createLegendNodes', 'usesBands', 'toSld', 'accept', 'inputBand', 'setInputBand']
    import functools as _functools
    __wrapped_QgsSingleBandPseudoColorRenderer_setShader = QgsSingleBandPseudoColorRenderer.setShader
    def __QgsSingleBandPseudoColorRenderer_setShader_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleBandPseudoColorRenderer_setShader(self, arg)
    QgsSingleBandPseudoColorRenderer.setShader = _functools.update_wrapper(__QgsSingleBandPseudoColorRenderer_setShader_wrapper, QgsSingleBandPseudoColorRenderer.setShader)

    import functools as _functools
    __wrapped_QgsSingleBandPseudoColorRenderer_createShader = QgsSingleBandPseudoColorRenderer.createShader
    def __QgsSingleBandPseudoColorRenderer_createShader_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSingleBandPseudoColorRenderer_createShader(self, arg)
    QgsSingleBandPseudoColorRenderer.createShader = _functools.update_wrapper(__QgsSingleBandPseudoColorRenderer_createShader_wrapper, QgsSingleBandPseudoColorRenderer.createShader)

    QgsSingleBandPseudoColorRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
