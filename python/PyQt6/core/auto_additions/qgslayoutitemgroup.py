# The following has been generated automatically from src/core/layout/qgslayoutitemgroup.h
try:
    QgsLayoutItemGroup.create = staticmethod(QgsLayoutItemGroup.create)
    QgsLayoutItemGroup.__overridden_methods__ = ['cleanup', 'type', 'displayName', 'setVisibility', 'attemptMove', 'attemptResize', 'paint', 'finalizeRestoreFromXml', 'exportLayerBehavior', 'rectWithFrame', 'draw', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    __wrapped_QgsLayoutItemGroup_addItem = QgsLayoutItemGroup.addItem
    def __QgsLayoutItemGroup_addItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemGroup_addItem(self, arg)
    QgsLayoutItemGroup.addItem = _functools.update_wrapper(__QgsLayoutItemGroup_addItem_wrapper, QgsLayoutItemGroup.addItem)

    QgsLayoutItemGroup.__group__ = ['layout']
except (NameError, AttributeError):
    pass
