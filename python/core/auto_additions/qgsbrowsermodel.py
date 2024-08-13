# The following has been generated automatically from src/core/browser/qgsbrowsermodel.h
QgsBrowserModel.ItemDataRole = QgsBrowserModel.CustomRole
# monkey patching scoped based enum
QgsBrowserModel.PathRole = QgsBrowserModel.CustomRole.Path
QgsBrowserModel.ItemDataRole.PathRole = QgsBrowserModel.CustomRole.Path
QgsBrowserModel.PathRole.is_monkey_patched = True
QgsBrowserModel.PathRole.__doc__ = "Item path used to access path in the tree, see QgsDataItem.mPath"
QgsBrowserModel.CommentRole = QgsBrowserModel.CustomRole.Comment
QgsBrowserModel.ItemDataRole.CommentRole = QgsBrowserModel.CustomRole.Comment
QgsBrowserModel.CommentRole.is_monkey_patched = True
QgsBrowserModel.CommentRole.__doc__ = "Item comment"
QgsBrowserModel.SortRole = QgsBrowserModel.CustomRole.Sort
QgsBrowserModel.ItemDataRole.SortRole = QgsBrowserModel.CustomRole.Sort
QgsBrowserModel.SortRole.is_monkey_patched = True
QgsBrowserModel.SortRole.__doc__ = "Custom sort role, see QgsDataItem.sortKey()"
QgsBrowserModel.ProviderKeyRole = QgsBrowserModel.CustomRole.ProviderKey
QgsBrowserModel.ItemDataRole.ProviderKeyRole = QgsBrowserModel.CustomRole.ProviderKey
QgsBrowserModel.ProviderKeyRole.is_monkey_patched = True
QgsBrowserModel.ProviderKeyRole.__doc__ = "Data item provider key that created the item, see QgsDataItem.providerKey() \n.. versionadded:: 3.12"
QgsBrowserModel.LayerMetadataRole = QgsBrowserModel.CustomRole.LayerMetadata
QgsBrowserModel.ItemDataRole.LayerMetadataRole = QgsBrowserModel.CustomRole.LayerMetadata
QgsBrowserModel.LayerMetadataRole.is_monkey_patched = True
QgsBrowserModel.LayerMetadataRole.__doc__ = ""
QgsBrowserModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsBrowserModel.ItemDataRole\n\n.. versionadded:: 3.36\n\n" + '* ``PathRole``: ' + QgsBrowserModel.CustomRole.Path.__doc__ + '\n' + '* ``CommentRole``: ' + QgsBrowserModel.CustomRole.Comment.__doc__ + '\n' + '* ``SortRole``: ' + QgsBrowserModel.CustomRole.Sort.__doc__ + '\n' + '* ``ProviderKeyRole``: ' + QgsBrowserModel.CustomRole.ProviderKey.__doc__ + '\n' + '* ``LayerMetadataRole``: ' + QgsBrowserModel.CustomRole.LayerMetadata.__doc__
# --
QgsBrowserModel.CustomRole.baseClass = QgsBrowserModel
QgsBrowserModel.__attribute_docs__ = {'stateChanged': 'Emitted when item children fetch was finished\n', 'connectionsChanged': 'Emitted when connections for the specified ``providerKey`` have changed in the browser.\n\nForwarded to the widget and used to notify the provider dialogs of a changed connection.\n'}
