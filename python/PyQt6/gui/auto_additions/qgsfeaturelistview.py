# The following has been generated automatically from src/gui/attributetable/qgsfeaturelistview.h
try:
    QgsFeatureListView.__attribute_docs__ = {'currentEditSelectionChanged': 'Emitted whenever the current edit selection has been changed.\n\n:param feat: the feature, which will be edited.\n', 'currentEditSelectionProgressChanged': 'Emitted whenever the current edit selection has been changed.\n\n:param progress: the position of the feature in the list\n:param count: the number of features in the list\n\n.. versionadded:: 3.8\n', 'displayExpressionChanged': 'Emitted whenever the display expression is successfully changed\n\n:param expression: The expression that was applied\n', 'willShowContextMenu': 'Emitted when the context menu is created to add the specific actions to it\n\n:param menu: is the already created context menu\n:param atIndex: is the position of the current feature in the model\n'}
    QgsFeatureListView.__signal_arguments__ = {'currentEditSelectionChanged': ['feat: QgsFeature'], 'currentEditSelectionProgressChanged': ['progress: int', 'count: int'], 'displayExpressionChanged': ['expression: str'], 'willShowContextMenu': ['menu: QgsActionMenu', 'atIndex: QModelIndex']}
    QgsFeatureListView.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
