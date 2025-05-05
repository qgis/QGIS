# The following has been generated automatically from src/core/annotations/qgsannotationlinetextitem.h
try:
    QgsAnnotationLineTextItem.create = staticmethod(QgsAnnotationLineTextItem.create)
    QgsAnnotationLineTextItem.__overridden_methods__ = ['flags', 'type', 'render', 'writeXml', 'nodesV2', 'applyEditV2', 'transientEditResultsV2', 'readXml', 'boundingBox', 'clone']
    import functools as _functools
    __wrapped_QgsAnnotationLineTextItem_setGeometry = QgsAnnotationLineTextItem.setGeometry
    def __QgsAnnotationLineTextItem_setGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationLineTextItem_setGeometry(self, arg)
    QgsAnnotationLineTextItem.setGeometry = _functools.update_wrapper(__QgsAnnotationLineTextItem_setGeometry_wrapper, QgsAnnotationLineTextItem.setGeometry)

    QgsAnnotationLineTextItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
