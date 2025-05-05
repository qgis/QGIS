# The following has been generated automatically from src/core/raster/qgsmultibandcolorrenderer.h
try:
    QgsMultiBandColorRenderer.create = staticmethod(QgsMultiBandColorRenderer.create)
    QgsMultiBandColorRenderer.__overridden_methods__ = ['clone', 'flags', 'block', 'writeXml', 'usesBands', 'createLegendNodes', 'toSld']
    import functools as _functools
    __wrapped_QgsMultiBandColorRenderer_setRedContrastEnhancement = QgsMultiBandColorRenderer.setRedContrastEnhancement
    def __QgsMultiBandColorRenderer_setRedContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiBandColorRenderer_setRedContrastEnhancement(self, arg)
    QgsMultiBandColorRenderer.setRedContrastEnhancement = _functools.update_wrapper(__QgsMultiBandColorRenderer_setRedContrastEnhancement_wrapper, QgsMultiBandColorRenderer.setRedContrastEnhancement)

    import functools as _functools
    __wrapped_QgsMultiBandColorRenderer_setGreenContrastEnhancement = QgsMultiBandColorRenderer.setGreenContrastEnhancement
    def __QgsMultiBandColorRenderer_setGreenContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiBandColorRenderer_setGreenContrastEnhancement(self, arg)
    QgsMultiBandColorRenderer.setGreenContrastEnhancement = _functools.update_wrapper(__QgsMultiBandColorRenderer_setGreenContrastEnhancement_wrapper, QgsMultiBandColorRenderer.setGreenContrastEnhancement)

    import functools as _functools
    __wrapped_QgsMultiBandColorRenderer_setBlueContrastEnhancement = QgsMultiBandColorRenderer.setBlueContrastEnhancement
    def __QgsMultiBandColorRenderer_setBlueContrastEnhancement_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMultiBandColorRenderer_setBlueContrastEnhancement(self, arg)
    QgsMultiBandColorRenderer.setBlueContrastEnhancement = _functools.update_wrapper(__QgsMultiBandColorRenderer_setBlueContrastEnhancement_wrapper, QgsMultiBandColorRenderer.setBlueContrastEnhancement)

    QgsMultiBandColorRenderer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
