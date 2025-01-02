# The following has been generated automatically from src/core/browser/qgsdataitem.h
try:
    QgsDataItem.__attribute_docs__ = {'beginInsertItems': 'Emitted before child items are added to this data item.\n\nThis signal *must* be followed by :py:func:`~QgsDataItem.endInsertItems`.\n\n:param parent: the parent item having children added, will always be this object\n:param first: index of first child item to be added\n:param last: index last child item, after the addition has occurred\n\n.. seealso:: :py:func:`endInsertItems`\n', 'endInsertItems': 'Emitted after child items have been added to this data item.\n\nThis signal will always be preceded by :py:func:`~QgsDataItem.beginInsertItems`.\n\n.. seealso:: :py:func:`beginInsertItems`\n', 'beginRemoveItems': 'Emitted before child items are removed from this data item.\n\nThis signal *must* be followed by :py:func:`~QgsDataItem.endRemoveItems`.\n\n:param parent: the parent item having children removed, will always be this object\n:param first: index of first child item to be removed\n:param last: index of the last child item to be removed\n\n.. seealso:: :py:func:`endRemoveItems`\n', 'endRemoveItems': 'Emitted after child items have been removed from this data item.\n\nThis signal will always be preceded by :py:func:`~QgsDataItem.beginRemoveItems`.\n\n.. seealso:: :py:func:`beginRemoveItems`\n', 'dataChanged': 'Emitted when data changes for an ``item``.\n', 'stateChanged': "Emitted when an item's state is changed.\n", 'connectionsChanged': 'Emitted when the connections of the provider with the specified ``providerKey`` have changed.\n\nThis signal is normally forwarded to the app in order to refresh the connection\nitem in the provider dialogs and to refresh the connection items in the other\nopen browsers.\n'}
    QgsDataItem.findItem = staticmethod(QgsDataItem.findItem)
    QgsDataItem.pathComponent = staticmethod(QgsDataItem.pathComponent)
    QgsDataItem.__signal_arguments__ = {'beginInsertItems': ['parent: QgsDataItem', 'first: int', 'last: int'], 'beginRemoveItems': ['parent: QgsDataItem', 'first: int', 'last: int'], 'dataChanged': ['item: QgsDataItem'], 'stateChanged': ['item: QgsDataItem', 'oldState: Qgis.BrowserItemState'], 'connectionsChanged': ['providerKey: Optional[str] = None']}
    QgsDataItem.__group__ = ['browser']
except (NameError, AttributeError):
    pass
try:
    QgsErrorItem.__group__ = ['browser']
except (NameError, AttributeError):
    pass
