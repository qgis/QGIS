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
QgsBrowserModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsBrowserModel.ItemDataRole

.. versionadded:: 3.36

* ``PathRole``: Item path used to access path in the tree, see QgsDataItem.mPath
* ``CommentRole``: Item comment
* ``SortRole``: Custom sort role, see QgsDataItem.sortKey()
* ``ProviderKeyRole``: Data item provider key that created the item, see QgsDataItem.providerKey()

  .. versionadded:: 3.12

* ``LayerMetadataRole``: 

"""
# --
QgsBrowserModel.CustomRole.baseClass = QgsBrowserModel
try:
    QgsBrowserModel.__attribute_docs__ = {'stateChanged': 'Emitted when item children fetch was finished\n', 'connectionsChanged': 'Emitted when connections for the specified ``providerKey`` have changed in the browser.\n\nForwarded to the widget and used to notify the provider dialogs of a changed connection.\n'}
except NameError:
    pass
try:
    QgsBrowserModel.__group__ = ['browser']
except NameError:
    pass
