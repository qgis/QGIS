# The following has been generated automatically from src/gui/layout/qgslayoutitemguiregistry.h
try:
    QgsLayoutItemGuiGroup.__attribute_docs__ = {'id': 'Unique (untranslated) group ID string.', 'name': 'Translated group name.', 'icon': 'Icon for group.'}
    QgsLayoutItemGuiGroup.__annotations__ = {'id': str, 'name': str, 'icon': 'QIcon'}
    QgsLayoutItemGuiGroup.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemGuiRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``metadataId``.\n', 'typeRemoved': 'Emitted whenever an item type is removed from the registry, with the\nspecified ``metadataId``.\n\n.. versionadded:: 4.0\n', 'groupRemoved': 'Emitted whenever an item group is removed from the registry.\n\n.. versionadded:: 4.0\n'}
    QgsLayoutItemGuiRegistry.__signal_arguments__ = {'typeAdded': ['metadataId: int'], 'typeRemoved': ['metadataId: int'], 'groupRemoved': ['groupId: str']}
    QgsLayoutItemGuiRegistry.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemAbstractGuiMetadata.__virtual_methods__ = ['creationIcon', 'createItemWidget', 'createRubberBand', 'createNodeRubberBand', 'createItem', 'newItemAddedToLayout', 'handleDoubleClick']
    QgsLayoutItemAbstractGuiMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
