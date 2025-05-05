# The following has been generated automatically from src/core/layout/qgslayoutitemregistry.h
try:
    QgsLayoutItemRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``type`` and visible ``name``.\n', 'multiFrameTypeAdded': 'Emitted whenever a new multiframe type is added to the registry, with\nthe specified ``type`` and visible ``name``.\n'}
    QgsLayoutItemRegistry.__signal_arguments__ = {'typeAdded': ['type: int', 'name: str'], 'multiFrameTypeAdded': ['type: int', 'name: str']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemRegistry_addLayoutItemType = QgsLayoutItemRegistry.addLayoutItemType
    def __QgsLayoutItemRegistry_addLayoutItemType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemRegistry_addLayoutItemType(self, arg)
    QgsLayoutItemRegistry.addLayoutItemType = _functools.update_wrapper(__QgsLayoutItemRegistry_addLayoutItemType_wrapper, QgsLayoutItemRegistry.addLayoutItemType)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemRegistry_addLayoutMultiFrameType = QgsLayoutItemRegistry.addLayoutMultiFrameType
    def __QgsLayoutItemRegistry_addLayoutMultiFrameType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemRegistry_addLayoutMultiFrameType(self, arg)
    QgsLayoutItemRegistry.addLayoutMultiFrameType = _functools.update_wrapper(__QgsLayoutItemRegistry_addLayoutMultiFrameType_wrapper, QgsLayoutItemRegistry.addLayoutMultiFrameType)

    QgsLayoutItemRegistry.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemAbstractMetadata.__virtual_methods__ = ['resolvePaths']
    QgsLayoutItemAbstractMetadata.__abstract_methods__ = ['createItem']
    QgsLayoutItemAbstractMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutMultiFrameAbstractMetadata.__virtual_methods__ = ['icon', 'resolvePaths']
    QgsLayoutMultiFrameAbstractMetadata.__abstract_methods__ = ['createMultiFrame']
    QgsLayoutMultiFrameAbstractMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
