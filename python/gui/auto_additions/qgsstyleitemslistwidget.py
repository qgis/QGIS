# The following has been generated automatically from src/gui/qgsstyleitemslistwidget.h
try:
    QgsStyleItemsListWidget.__attribute_docs__ = {'selectionChanged': 'Emitted when the selected item is changed in the widget.\n\n:param name: Newly selected item name\n:param type: Newly selected item type\n', 'selectionChangedWithStylePath': 'Emitted when the selected item is changed in the widget.\n\n:param name: Newly selected item name\n:param type: Newly selected item type\n:param stylePath: file path to associated style database\n\n.. versionadded:: 3.26\n', 'saveEntity': 'Emitted when the user has opted to save a new entity to the style\ndatabase, by clicking the "Save" button in the widget.\n\nIt is the caller\'s responsibility to handle this in an appropriate\nmanner given the context of the widget.\n'}
    QgsStyleItemsListWidget.__signal_arguments__ = {'selectionChanged': ['name: str', 'type: QgsStyle.StyleEntity'], 'selectionChangedWithStylePath': ['name: str', 'type: QgsStyle.StyleEntity', 'stylePath: str']}
except (NameError, AttributeError):
    pass
