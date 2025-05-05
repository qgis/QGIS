# The following has been generated automatically from src/core/annotations/qgsannotationrectitem.h
try:
    QgsAnnotationRectItem.__abstract_methods__ = ['renderInBounds']
    QgsAnnotationRectItem.__overridden_methods__ = ['flags', 'render', 'nodesV2', 'applyEditV2', 'transientEditResultsV2', 'boundingBox', 'copyCommonProperties', 'writeCommonProperties', 'readCommonProperties']
    import functools as _functools
    __wrapped_QgsAnnotationRectItem_setBackgroundSymbol = QgsAnnotationRectItem.setBackgroundSymbol
    def __QgsAnnotationRectItem_setBackgroundSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationRectItem_setBackgroundSymbol(self, arg)
    QgsAnnotationRectItem.setBackgroundSymbol = _functools.update_wrapper(__QgsAnnotationRectItem_setBackgroundSymbol_wrapper, QgsAnnotationRectItem.setBackgroundSymbol)

    import functools as _functools
    __wrapped_QgsAnnotationRectItem_setFrameSymbol = QgsAnnotationRectItem.setFrameSymbol
    def __QgsAnnotationRectItem_setFrameSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationRectItem_setFrameSymbol(self, arg)
    QgsAnnotationRectItem.setFrameSymbol = _functools.update_wrapper(__QgsAnnotationRectItem_setFrameSymbol_wrapper, QgsAnnotationRectItem.setFrameSymbol)

    QgsAnnotationRectItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
