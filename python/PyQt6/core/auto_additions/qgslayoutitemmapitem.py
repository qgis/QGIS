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
    import functools as _functools
    __wrapped_QgsLayoutItemMapItemStack_addItem = QgsLayoutItemMapItemStack.addItem
    def __QgsLayoutItemMapItemStack_addItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapItemStack_addItem(self, arg)
    QgsLayoutItemMapItemStack.addItem = _functools.update_wrapper(__QgsLayoutItemMapItemStack_addItem_wrapper, QgsLayoutItemMapItemStack.addItem)

    QgsLayoutItemMapItemStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
