# The following has been generated automatically from src/core/layout/qgslayouttable.h
try:
    QgsLayoutTableStyle.__attribute_docs__ = {'enabled': 'Whether the styling option is enabled', 'cellBackgroundColor': 'Cell background color'}
    QgsLayoutTableStyle.__annotations__ = {'enabled': bool, 'cellBackgroundColor': 'QColor'}
    QgsLayoutTableStyle.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutTable.__virtual_methods__ = ['conditionalCellStyle', 'scopeForCell', 'rowSpan', 'columnSpan', 'refreshAttributes', 'calculateMaxColumnWidths', 'calculateMaxRowHeights', 'textFormatForCell', 'textFormatForHeader', 'horizontalAlignmentForCell', 'verticalAlignmentForCell']
    QgsLayoutTable.__abstract_methods__ = ['getTableContents']
    QgsLayoutTable.__overridden_methods__ = ['fixedFrameSize', 'minFrameSize', 'writePropertiesToElement', 'readPropertiesFromElement', 'totalSize', 'render', 'refresh', 'recalculateFrameSizes']
    QgsLayoutTable.__group__ = ['layout']
except (NameError, AttributeError):
    pass
