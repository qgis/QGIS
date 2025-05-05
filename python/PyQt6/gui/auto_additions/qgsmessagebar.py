# The following has been generated automatically from src/gui/qgsmessagebar.h
try:
    QgsMessageBar.__attribute_docs__ = {'widgetAdded': 'Emitted whenever an ``item`` is added to the bar.\n', 'widgetRemoved': 'Emitted whenever an ``item`` was removed from the bar.\n'}
    QgsMessageBar.createMessage = staticmethod(QgsMessageBar.createMessage)
    QgsMessageBar.defaultMessageTimeout = staticmethod(QgsMessageBar.defaultMessageTimeout)
    QgsMessageBar.__overridden_methods__ = ['mousePressEvent']
    QgsMessageBar.__signal_arguments__ = {'widgetAdded': ['item: QgsMessageBarItem'], 'widgetRemoved': ['item: QgsMessageBarItem']}
    import functools as _functools
    __wrapped_QgsMessageBar_pushItem = QgsMessageBar.pushItem
    def __QgsMessageBar_pushItem_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsMessageBar_pushItem(self, arg)
    QgsMessageBar.pushItem = _functools.update_wrapper(__QgsMessageBar_pushItem_wrapper, QgsMessageBar.pushItem)

except (NameError, AttributeError):
    pass
