# The following has been generated automatically from src/gui/annotations/qgsannotationitemguiregistry.h
try:
    QgsAnnotationItemGuiGroup.__attribute_docs__ = {'id': 'Unique (untranslated) group ID string.', 'name': 'Translated group name.', 'icon': 'Icon for group.'}
    QgsAnnotationItemGuiGroup.__annotations__ = {'id': str, 'name': str, 'icon': 'QIcon'}
    QgsAnnotationItemGuiGroup.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemGuiRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the\nspecified ``metadataId``.\n'}
    QgsAnnotationItemGuiRegistry.__signal_arguments__ = {'typeAdded': ['metadataId: int']}
    QgsAnnotationItemGuiRegistry.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
try:
    QgsAnnotationItemAbstractGuiMetadata.__virtual_methods__ = ['creationIcon', 'createItemWidget', 'createMapTool', 'createItem', 'newItemAddedToLayer']
    QgsAnnotationItemAbstractGuiMetadata.__group__ = ['annotations']
except (NameError, AttributeError):
    pass
