# The following has been generated automatically from src/gui/layout/qgslayoutitemguiregistry.h
try:
    QgsLayoutItemGuiGroup.__attribute_docs__ = {'id': 'Unique (untranslated) group ID string.', 'name': 'Translated group name.', 'icon': 'Icon for group.'}
    QgsLayoutItemGuiGroup.__annotations__ = {'id': str, 'name': str, 'icon': 'QIcon'}
    QgsLayoutItemGuiGroup.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemGuiRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``metadataId``.\n'}
    QgsLayoutItemGuiRegistry.__signal_arguments__ = {'typeAdded': ['metadataId: int']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemGuiRegistry_addLayoutItemGuiMetadata = QgsLayoutItemGuiRegistry.addLayoutItemGuiMetadata
    def __QgsLayoutItemGuiRegistry_addLayoutItemGuiMetadata_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemGuiRegistry_addLayoutItemGuiMetadata(self, arg)
    QgsLayoutItemGuiRegistry.addLayoutItemGuiMetadata = _functools.update_wrapper(__QgsLayoutItemGuiRegistry_addLayoutItemGuiMetadata_wrapper, QgsLayoutItemGuiRegistry.addLayoutItemGuiMetadata)

    QgsLayoutItemGuiRegistry.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemAbstractGuiMetadata.__virtual_methods__ = ['creationIcon', 'createItemWidget', 'createRubberBand', 'createNodeRubberBand', 'createItem', 'newItemAddedToLayout', 'handleDoubleClick']
    QgsLayoutItemAbstractGuiMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
