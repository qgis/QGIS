# The following has been generated automatically from src/core/layout/qgslayoutitemregistry.h
QgsLayoutItemRegistry.LayoutItem = QgsLayoutItemRegistry.ItemType.LayoutItem
QgsLayoutItemRegistry.LayoutGroup = QgsLayoutItemRegistry.ItemType.LayoutGroup
QgsLayoutItemRegistry.LayoutPage = QgsLayoutItemRegistry.ItemType.LayoutPage
QgsLayoutItemRegistry.LayoutMap = QgsLayoutItemRegistry.ItemType.LayoutMap
QgsLayoutItemRegistry.LayoutPicture = QgsLayoutItemRegistry.ItemType.LayoutPicture
QgsLayoutItemRegistry.LayoutLabel = QgsLayoutItemRegistry.ItemType.LayoutLabel
QgsLayoutItemRegistry.LayoutLegend = QgsLayoutItemRegistry.ItemType.LayoutLegend
QgsLayoutItemRegistry.LayoutShape = QgsLayoutItemRegistry.ItemType.LayoutShape
QgsLayoutItemRegistry.LayoutPolygon = QgsLayoutItemRegistry.ItemType.LayoutPolygon
QgsLayoutItemRegistry.LayoutPolyline = QgsLayoutItemRegistry.ItemType.LayoutPolyline
QgsLayoutItemRegistry.LayoutScaleBar = QgsLayoutItemRegistry.ItemType.LayoutScaleBar
QgsLayoutItemRegistry.LayoutFrame = QgsLayoutItemRegistry.ItemType.LayoutFrame
QgsLayoutItemRegistry.LayoutHtml = QgsLayoutItemRegistry.ItemType.LayoutHtml
QgsLayoutItemRegistry.LayoutAttributeTable = QgsLayoutItemRegistry.ItemType.LayoutAttributeTable
QgsLayoutItemRegistry.LayoutTextTable = QgsLayoutItemRegistry.ItemType.LayoutTextTable
QgsLayoutItemRegistry.Layout3DMap = QgsLayoutItemRegistry.ItemType.Layout3DMap
QgsLayoutItemRegistry.LayoutManualTable = QgsLayoutItemRegistry.ItemType.LayoutManualTable
QgsLayoutItemRegistry.LayoutMarker = QgsLayoutItemRegistry.ItemType.LayoutMarker
QgsLayoutItemRegistry.LayoutElevationProfile = QgsLayoutItemRegistry.ItemType.LayoutElevationProfile
QgsLayoutItemRegistry.PluginItem = QgsLayoutItemRegistry.ItemType.PluginItem
try:
    QgsLayoutItemRegistry.__attribute_docs__ = {'typeAdded': 'Emitted whenever a new item type is added to the registry, with the specified\n``type`` and visible ``name``.\n', 'multiFrameTypeAdded': 'Emitted whenever a new multiframe type is added to the registry, with the specified\n``type`` and visible ``name``.\n'}
    QgsLayoutItemRegistry.__signal_arguments__ = {'typeAdded': ['type: int', 'name: str'], 'multiFrameTypeAdded': ['type: int', 'name: str']}
    QgsLayoutItemRegistry.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemAbstractMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutMultiFrameAbstractMetadata.__group__ = ['layout']
except (NameError, AttributeError):
    pass
