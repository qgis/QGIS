# The following has been generated automatically from src/core/annotations/qgsannotationpolygonitem.h
try:
    QgsAnnotationPolygonItem.create = staticmethod(QgsAnnotationPolygonItem.create)
    QgsAnnotationPolygonItem.__overridden_methods__ = ['type', 'render', 'writeXml', 'nodesV2', 'applyEditV2', 'transientEditResultsV2', 'flags', 'readXml', 'clone', 'boundingBox']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationPolygonItem_QgsAnnotationPolygonItem = QgsAnnotationPolygonItem.QgsAnnotationPolygonItem
    def __QgsAnnotationPolygonItem_QgsAnnotationPolygonItem_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationPolygonItem_QgsAnnotationPolygonItem(self, arg)
    QgsAnnotationPolygonItem.QgsAnnotationPolygonItem = _functools.update_wrapper(__QgsAnnotationPolygonItem_QgsAnnotationPolygonItem_wrapper, QgsAnnotationPolygonItem.QgsAnnotationPolygonItem)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationPolygonItem_setGeometry = QgsAnnotationPolygonItem.setGeometry
    def __QgsAnnotationPolygonItem_setGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationPolygonItem_setGeometry(self, arg)
    QgsAnnotationPolygonItem.setGeometry = _functools.update_wrapper(__QgsAnnotationPolygonItem_setGeometry_wrapper, QgsAnnotationPolygonItem.setGeometry)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationPolygonItem_setSymbol = QgsAnnotationPolygonItem.setSymbol
    def __QgsAnnotationPolygonItem_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationPolygonItem_setSymbol(self, arg)
    QgsAnnotationPolygonItem.setSymbol = _functools.update_wrapper(__QgsAnnotationPolygonItem_setSymbol_wrapper, QgsAnnotationPolygonItem.setSymbol)

    QgsAnnotationPolygonItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
