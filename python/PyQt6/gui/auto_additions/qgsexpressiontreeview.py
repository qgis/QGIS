# The following has been generated automatically from src/gui/qgsexpressiontreeview.h
QgsExpressionItem.Header = QgsExpressionItem.ItemType.Header
QgsExpressionItem.Field = QgsExpressionItem.ItemType.Field
QgsExpressionItem.ExpressionNode = QgsExpressionItem.ItemType.ExpressionNode
try:
    QgsExpressionItem.__attribute_docs__ = {'CUSTOM_SORT_ROLE': 'Custom sort order role', 'ITEM_TYPE_ROLE': 'Item type role', 'SEARCH_TAGS_ROLE': 'Search tags role', 'ITEM_NAME_ROLE': 'Item name role', 'LAYER_ID_ROLE': '\n.. versionadded:: 3.24'}
    QgsExpressionItem.__annotations__ = {'CUSTOM_SORT_ROLE': int, 'ITEM_TYPE_ROLE': int, 'SEARCH_TAGS_ROLE': int, 'ITEM_NAME_ROLE': int, 'LAYER_ID_ROLE': int}
except (NameError, AttributeError):
    pass
try:
    QgsExpressionTreeView.__attribute_docs__ = {'expressionItemDoubleClicked': 'Emitted when a expression item is double clicked\n', 'currentExpressionItemChanged': 'Emitter when the current expression item changed\n'}
    QgsExpressionTreeView.__signal_arguments__ = {'expressionItemDoubleClicked': ['text: str'], 'currentExpressionItemChanged': ['item: QgsExpressionItem']}
except (NameError, AttributeError):
    pass
try:
    QgsExpressionTreeView.MenuProvider.__virtual_methods__ = ['createContextMenu']
except (NameError, AttributeError):
    pass
try:
    QgsExpressionItemSearchProxy.__overridden_methods__ = ['filterAcceptsRow', 'lessThan']
except (NameError, AttributeError):
    pass
