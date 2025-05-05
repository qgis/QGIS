# The following has been generated automatically from src/core/vector/qgsvectorlayerelevationproperties.h
try:
    QgsVectorLayerElevationProperties.__overridden_methods__ = ['hasElevation', 'writeXml', 'readXml', 'setDefaultsFromLayer', 'clone', 'htmlSummary', 'isVisibleInZRange', 'calculateZRange', 'showByDefaultInElevationProfilePlots']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsVectorLayerElevationProperties_setProfileLineSymbol = QgsVectorLayerElevationProperties.setProfileLineSymbol
    def __QgsVectorLayerElevationProperties_setProfileLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerElevationProperties_setProfileLineSymbol(self, arg)
    QgsVectorLayerElevationProperties.setProfileLineSymbol = _functools.update_wrapper(__QgsVectorLayerElevationProperties_setProfileLineSymbol_wrapper, QgsVectorLayerElevationProperties.setProfileLineSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsVectorLayerElevationProperties_setProfileFillSymbol = QgsVectorLayerElevationProperties.setProfileFillSymbol
    def __QgsVectorLayerElevationProperties_setProfileFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerElevationProperties_setProfileFillSymbol(self, arg)
    QgsVectorLayerElevationProperties.setProfileFillSymbol = _functools.update_wrapper(__QgsVectorLayerElevationProperties_setProfileFillSymbol_wrapper, QgsVectorLayerElevationProperties.setProfileFillSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsVectorLayerElevationProperties_setProfileMarkerSymbol = QgsVectorLayerElevationProperties.setProfileMarkerSymbol
    def __QgsVectorLayerElevationProperties_setProfileMarkerSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsVectorLayerElevationProperties_setProfileMarkerSymbol(self, arg)
    QgsVectorLayerElevationProperties.setProfileMarkerSymbol = _functools.update_wrapper(__QgsVectorLayerElevationProperties_setProfileMarkerSymbol_wrapper, QgsVectorLayerElevationProperties.setProfileMarkerSymbol)

    QgsVectorLayerElevationProperties.__group__ = ['vector']
except (NameError, AttributeError):
    pass
