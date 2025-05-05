# The following has been generated automatically from src/core/raster/qgsrasterlayerelevationproperties.h
try:
    QgsRasterLayerElevationProperties.layerLooksLikeDem = staticmethod(QgsRasterLayerElevationProperties.layerLooksLikeDem)
    QgsRasterLayerElevationProperties.__overridden_methods__ = ['hasElevation', 'writeXml', 'readXml', 'clone', 'htmlSummary', 'isVisibleInZRange', 'calculateZRange', 'significantZValues', 'showByDefaultInElevationProfilePlots', 'flags']
    import functools as _functools
    __wrapped_QgsRasterLayerElevationProperties_setProfileLineSymbol = QgsRasterLayerElevationProperties.setProfileLineSymbol
    def __QgsRasterLayerElevationProperties_setProfileLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterLayerElevationProperties_setProfileLineSymbol(self, arg)
    QgsRasterLayerElevationProperties.setProfileLineSymbol = _functools.update_wrapper(__QgsRasterLayerElevationProperties_setProfileLineSymbol_wrapper, QgsRasterLayerElevationProperties.setProfileLineSymbol)

    import functools as _functools
    __wrapped_QgsRasterLayerElevationProperties_setProfileFillSymbol = QgsRasterLayerElevationProperties.setProfileFillSymbol
    def __QgsRasterLayerElevationProperties_setProfileFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsRasterLayerElevationProperties_setProfileFillSymbol(self, arg)
    QgsRasterLayerElevationProperties.setProfileFillSymbol = _functools.update_wrapper(__QgsRasterLayerElevationProperties_setProfileFillSymbol_wrapper, QgsRasterLayerElevationProperties.setProfileFillSymbol)

    QgsRasterLayerElevationProperties.__group__ = ['raster']
except (NameError, AttributeError):
    pass
