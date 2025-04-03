# The following has been generated automatically from src/gui/attributetable/qgsattributetableview.h
try:
    QgsAttributeTableView.__attribute_docs__ = {'willShowContextMenu': 'Emitted in order to provide a hook to add additional* menu entries to\nthe context menu.\n\n:param menu: If additional QMenuItems are added, they will show up in\n             the context menu.\n:param atIndex: The QModelIndex, to which the context menu belongs.\n                Relative to the source model. In most cases, this will\n                be a :py:class:`QgsAttributeTableFilterModel`\n', 'columnResized': 'Emitted when a column in the view has been resized.\n\n:param column: column index (starts at 0)\n:param width: new width in pixel\n', 'finished': '.. deprecated:: 3.40\n\n   No longer used.\n'}
    QgsAttributeTableView.__signal_arguments__ = {'willShowContextMenu': ['menu: QMenu', 'atIndex: QModelIndex'], 'columnResized': ['column: int', 'width: int']}
    QgsAttributeTableView.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
