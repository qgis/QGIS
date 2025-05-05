# The following has been generated automatically from src/core/mesh/qgsmeshlayerelevationproperties.h
try:
    QgsMeshLayerElevationProperties.__overridden_methods__ = ['hasElevation', 'writeXml', 'readXml', 'htmlSummary', 'clone', 'isVisibleInZRange', 'calculateZRange', 'significantZValues', 'showByDefaultInElevationProfilePlots', 'flags']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMeshLayerElevationProperties_setProfileLineSymbol = QgsMeshLayerElevationProperties.setProfileLineSymbol
    def __QgsMeshLayerElevationProperties_setProfileLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMeshLayerElevationProperties_setProfileLineSymbol(self, arg)
    QgsMeshLayerElevationProperties.setProfileLineSymbol = _functools.update_wrapper(__QgsMeshLayerElevationProperties_setProfileLineSymbol_wrapper, QgsMeshLayerElevationProperties.setProfileLineSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsMeshLayerElevationProperties_setProfileFillSymbol = QgsMeshLayerElevationProperties.setProfileFillSymbol
    def __QgsMeshLayerElevationProperties_setProfileFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMeshLayerElevationProperties_setProfileFillSymbol(self, arg)
    QgsMeshLayerElevationProperties.setProfileFillSymbol = _functools.update_wrapper(__QgsMeshLayerElevationProperties_setProfileFillSymbol_wrapper, QgsMeshLayerElevationProperties.setProfileFillSymbol)

    QgsMeshLayerElevationProperties.__group__ = ['mesh']
except (NameError, AttributeError):
    pass
