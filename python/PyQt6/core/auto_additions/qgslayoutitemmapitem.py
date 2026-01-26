# The following has been generated automatically from src/core/layout/qgslayoutitemmapitem.h
QgsLayoutItemMapItem.StackBelowMap = QgsLayoutItemMapItem.StackingPosition.StackBelowMap
QgsLayoutItemMapItem.StackBelowMapLayer = QgsLayoutItemMapItem.StackingPosition.StackBelowMapLayer
QgsLayoutItemMapItem.StackAboveMapLayer = QgsLayoutItemMapItem.StackingPosition.StackAboveMapLayer
QgsLayoutItemMapItem.StackBelowMapLabels = QgsLayoutItemMapItem.StackingPosition.StackBelowMapLabels
QgsLayoutItemMapItem.StackAboveMapLabels = QgsLayoutItemMapItem.StackingPosition.StackAboveMapLabels
try:
    QgsLayoutItemMapItem.__virtual_methods__ = ['writeXml', 'readXml', 'finalizeRestoreFromXml', 'setEnabled', 'usesAdvancedEffects', 'accept', 'mapLayer']
    QgsLayoutItemMapItem.__abstract_methods__ = ['draw']
    QgsLayoutItemMapItem.__overridden_methods__ = ['createExpressionContext']
    QgsLayoutItemMapItem.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMapItemStack.__virtual_methods__ = ['writeXml', 'finalizeRestoreFromXml']
    QgsLayoutItemMapItemStack.__abstract_methods__ = ['readXml']
    QgsLayoutItemMapItemStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
