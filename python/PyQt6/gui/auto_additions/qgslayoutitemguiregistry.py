# The following has been generated automatically from src/gui/layout/qgslayoutitemguiregistry.h
QgsLayoutItemAbstractGuiMetadata.FlagNoCreationTools = QgsLayoutItemAbstractGuiMetadata.Flag.FlagNoCreationTools
QgsLayoutItemAbstractGuiMetadata.Flags = lambda flags=0: QgsLayoutItemAbstractGuiMetadata.Flag(flags)
try:
    QgsLayoutItemGuiGroup.__attribute_docs__ = {'id': 'Unique (untranslated) group ID string.', 'name': 'Translated group name.', 'icon': 'Icon for group.'}
    QgsLayoutItemGuiGroup.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemGuiRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the specified\n``metadataId``.\n'}
    QgsLayoutItemGuiRegistry.__signal_arguments__ = {'typeAdded': ['metadataId: int']}
    QgsLayoutItemGuiRegistry.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemAbstractGuiMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
