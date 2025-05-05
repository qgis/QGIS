# The following has been generated automatically from src/core/annotations/qgsannotationlineitem.h
try:
    QgsAnnotationLineItem.create = staticmethod(QgsAnnotationLineItem.create)
    QgsAnnotationLineItem.__overridden_methods__ = ['type', 'render', 'writeXml', 'nodesV2', 'applyEditV2', 'transientEditResultsV2', 'flags', 'readXml', 'boundingBox', 'clone']
    import functools as _functools
    __wrapped_QgsAnnotationLineItem_QgsAnnotationLineItem = QgsAnnotationLineItem.QgsAnnotationLineItem
    def __QgsAnnotationLineItem_QgsAnnotationLineItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLineItem_QgsAnnotationLineItem(self, arg)
    QgsAnnotationLineItem.QgsAnnotationLineItem = _functools.update_wrapper(__QgsAnnotationLineItem_QgsAnnotationLineItem_wrapper, QgsAnnotationLineItem.QgsAnnotationLineItem)

    import functools as _functools
    __wrapped_QgsAnnotationLineItem_setGeometry = QgsAnnotationLineItem.setGeometry
    def __QgsAnnotationLineItem_setGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLineItem_setGeometry(self, arg)
    QgsAnnotationLineItem.setGeometry = _functools.update_wrapper(__QgsAnnotationLineItem_setGeometry_wrapper, QgsAnnotationLineItem.setGeometry)

    import functools as _functools
    __wrapped_QgsAnnotationLineItem_setSymbol = QgsAnnotationLineItem.setSymbol
    def __QgsAnnotationLineItem_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLineItem_setSymbol(self, arg)
    QgsAnnotationLineItem.setSymbol = _functools.update_wrapper(__QgsAnnotationLineItem_setSymbol_wrapper, QgsAnnotationLineItem.setSymbol)

    QgsAnnotationLineItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
