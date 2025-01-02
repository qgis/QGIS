# The following has been generated automatically from src/core/layout/qgslayout.h
try:
    QgsLayout.__attribute_docs__ = {'changed': 'Emitted when properties of the layout change. This signal is only\nemitted for settings directly managed by the layout, and is not emitted\nwhen child items change.\n', 'variablesChanged': 'Emitted whenever the expression variables stored in the layout have been changed.\n', 'selectedItemChanged': 'Emitted whenever the selected item changes.\nIf ``None``, no item is selected.\n', 'refreshed': 'Emitted when the layout has been refreshed and items should also be refreshed\nand updated.\n', 'backgroundTaskCountChanged': 'Emitted whenever the ``total`` number of background tasks running in items from the layout changes.\n\n.. versionadded:: 3.10\n', 'itemAdded': 'Emitted when an ``item`` was added to the layout.\n\n.. versionadded:: 3.20\n'}
    QgsLayout.__signal_arguments__ = {'selectedItemChanged': ['selected: QgsLayoutItem'], 'backgroundTaskCountChanged': ['total: int'], 'itemAdded': ['item: QgsLayoutItem']}
    QgsLayout.__group__ = ['layout']
except (NameError, AttributeError):
    pass
