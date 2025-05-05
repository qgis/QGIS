# The following has been generated automatically from src/core/annotations/qgsannotationitem.h
try:
    QgsAnnotationItem.__virtual_methods__ = ['flags', 'applyEdit', 'applyEditV2', 'transientEditResults', 'transientEditResultsV2', 'nodes', 'nodesV2', 'copyCommonProperties', 'writeCommonProperties', 'readCommonProperties']
    QgsAnnotationItem.__abstract_methods__ = ['clone', 'type', 'boundingBox', 'render', 'writeXml', 'readXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationItem_setCallout = QgsAnnotationItem.setCallout
    def __QgsAnnotationItem_setCallout_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationItem_setCallout(self, arg)
    QgsAnnotationItem.setCallout = _functools.update_wrapper(__QgsAnnotationItem_setCallout_wrapper, QgsAnnotationItem.setCallout)

    QgsAnnotationItem.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
