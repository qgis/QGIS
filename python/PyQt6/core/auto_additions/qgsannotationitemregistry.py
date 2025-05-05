# The following has been generated automatically from src/core/annotations/qgsannotationitemregistry.h
try:
    QgsAnnotationItemRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``type`` and visible ``name``.\n'}
    QgsAnnotationItemRegistry.__signal_arguments__ = {'typeAdded': ['type: str', 'name: str']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAnnotationItemRegistry_addItemType = QgsAnnotationItemRegistry.addItemType
    def __QgsAnnotationItemRegistry_addItemType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAnnotationItemRegistry_addItemType(self, arg)
    QgsAnnotationItemRegistry.addItemType = _functools.update_wrapper(__QgsAnnotationItemRegistry_addItemType_wrapper, QgsAnnotationItemRegistry.addItemType)

    QgsAnnotationItemRegistry.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemAbstractMetadata.__abstract_methods__ = ['createItem']
    QgsAnnotationItemAbstractMetadata.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
