# The following has been generated automatically from src/gui/annotations/qgscreateannotationitemmaptool.h
try:
    QgsCreateAnnotationItemMapToolHandler.__attribute_docs__ = {'itemCreated': 'Emitted by the tool when a new annotation item has been created.\n\nClients should connect to this signal and call\n:py:func:`~QgsCreateAnnotationItemMapToolHandler.takeCreatedItem` to\ntake the newly created item from the map tool.\n'}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsCreateAnnotationItemMapToolHandler_pushCreatedItem = QgsCreateAnnotationItemMapToolHandler.pushCreatedItem
    def __QgsCreateAnnotationItemMapToolHandler_pushCreatedItem_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsCreateAnnotationItemMapToolHandler_pushCreatedItem(self, arg)
    QgsCreateAnnotationItemMapToolHandler.pushCreatedItem = _functools.update_wrapper(__QgsCreateAnnotationItemMapToolHandler_pushCreatedItem_wrapper, QgsCreateAnnotationItemMapToolHandler.pushCreatedItem)

    QgsCreateAnnotationItemMapToolHandler.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsCreateAnnotationItemMapToolInterface.__abstract_methods__ = ['handler', 'mapTool']
    QgsCreateAnnotationItemMapToolInterface.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
