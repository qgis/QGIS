# The following has been generated automatically from src/core/annotations/qgsannotationmarkeritem.h
try:
    QgsAnnotationMarkerItem.create = staticmethod(QgsAnnotationMarkerItem.create)
    QgsAnnotationMarkerItem.__overridden_methods__ = ['type', 'render', 'writeXml', 'flags', 'nodesV2', 'applyEditV2', 'transientEditResultsV2', 'readXml', 'clone', 'boundingBox']
    import functools as _functools
    __wrapped_QgsAnnotationMarkerItem_setSymbol = QgsAnnotationMarkerItem.setSymbol
    def __QgsAnnotationMarkerItem_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationMarkerItem_setSymbol(self, arg)
    QgsAnnotationMarkerItem.setSymbol = _functools.update_wrapper(__QgsAnnotationMarkerItem_setSymbol_wrapper, QgsAnnotationMarkerItem.setSymbol)

    QgsAnnotationMarkerItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
