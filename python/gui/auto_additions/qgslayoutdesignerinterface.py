# The following has been generated automatically from src/gui/layout/qgslayoutdesignerinterface.h
try:
    QgsLayoutDesignerInterface.ExportResults.__attribute_docs__ = {'result': 'Result/error code of export.', 'labelingResults': 'Returns the labeling results for all map items included in the export. Map keys are the item UUIDs (see :py:func:`QgsLayoutItem.uuid()`).\n\nOwnership of the results remains with the layout designer.'}
    QgsLayoutDesignerInterface.ExportResults.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutDesignerInterface.__attribute_docs__ = {'layoutExported': 'Emitted whenever a layout is exported from the layout designer.\n\nThe results of the export can be retrieved by calling :py:func:`~QgsLayoutDesignerInterface.lastExportResults`.\n\n.. versionadded:: 3.20\n', 'mapPreviewRefreshed': "Emitted when a ``map`` item's preview has been refreshed.\n\n.. versionadded:: 3.20\n"}
    QgsLayoutDesignerInterface.__signal_arguments__ = {'mapPreviewRefreshed': ['map: QgsLayoutItemMap']}
    QgsLayoutDesignerInterface.__group__ = ['layout']
except (NameError, AttributeError):
    pass
