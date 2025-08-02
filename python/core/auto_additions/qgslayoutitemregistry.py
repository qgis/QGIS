# The following has been generated automatically from src/core/layout/qgslayoutitemregistry.h
try:
    QgsLayoutItemRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``type`` and visible ``name``.\n', 'typeRemoved': 'Emitted whenever an item type is removed from the registry with the\nspecified ``type``.\n\n.. versionadded:: 4.0\n', 'multiFrameTypeAdded': 'Emitted whenever a new multiframe type is added to the registry, with\nthe specified ``type`` and visible ``name``.\n', 'multiFrameTypeRemoved': 'Emitted whenever an multiframe type is removed from the registry with\nthe specified ``type``.\n\n.. versionadded:: 4.0\n'}
    QgsLayoutItemRegistry.__signal_arguments__ = {'typeAdded': ['type: int', 'name: str'], 'typeRemoved': ['type: int'], 'multiFrameTypeAdded': ['type: int', 'name: str'], 'multiFrameTypeRemoved': ['type: int']}
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
