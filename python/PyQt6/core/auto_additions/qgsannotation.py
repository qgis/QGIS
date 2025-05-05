# The following has been generated automatically from src/core/annotations/qgsannotation.h
try:
    QgsAnnotation.__attribute_docs__ = {'appearanceChanged': "Emitted whenever the annotation's appearance changes\n", 'moved': "Emitted when the annotation's position has changed and items need to be\nmoved to reflect this.\n", 'mapLayerChanged': 'Emitted when the map layer associated with the annotation changes.\n'}
    QgsAnnotation.__virtual_methods__ = ['setAssociatedFeature', 'accept', 'minimumFrameSize']
    QgsAnnotation.__abstract_methods__ = ['clone', 'writeXml', 'readXml', 'renderAnnotation']
    import functools as _functools
    __wrapped_QgsAnnotation_setFillSymbol = QgsAnnotation.setFillSymbol
    def __QgsAnnotation_setFillSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotation_setFillSymbol(self, arg)
    QgsAnnotation.setFillSymbol = _functools.update_wrapper(__QgsAnnotation_setFillSymbol_wrapper, QgsAnnotation.setFillSymbol)

    import functools as _functools
    __wrapped_QgsAnnotation_setMarkerSymbol = QgsAnnotation.setMarkerSymbol
    def __QgsAnnotation_setMarkerSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotation_setMarkerSymbol(self, arg)
    QgsAnnotation.setMarkerSymbol = _functools.update_wrapper(__QgsAnnotation_setMarkerSymbol_wrapper, QgsAnnotation.setMarkerSymbol)

    QgsAnnotation.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
