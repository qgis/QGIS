# The following has been generated automatically from src/gui/history/qgshistoryentrynode.h
try:
    QgsHistoryEntryNode.__virtual_methods__ = ['childCount', 'html', 'createWidget', 'doubleClicked', 'populateContextMenu', 'matchesString']
    QgsHistoryEntryNode.__abstract_methods__ = ['data']
    QgsHistoryEntryNode.__group__ = ['history']
except (NameError, AttributeError):
    pass
try:
    QgsHistoryEntryGroup.__virtual_methods__ = ['childCount']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsHistoryEntryGroup_addChild = QgsHistoryEntryGroup.addChild
    def __QgsHistoryEntryGroup_addChild_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsHistoryEntryGroup_addChild(self, arg)
    QgsHistoryEntryGroup.addChild = _functools.update_wrapper(__QgsHistoryEntryGroup_addChild_wrapper, QgsHistoryEntryGroup.addChild)

    QgsHistoryEntryGroup.__group__ = ['history']
except (NameError, AttributeError):
    pass
