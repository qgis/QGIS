# The following has been generated automatically from src/gui/processing/qgsprocessingtoolboxtreeview.h
try:
    QgsProcessingToolboxTreeView.__overridden_methods__ = ['keyPressEvent']
    import functools as _functools
    __wrapped_QgsProcessingToolboxTreeView_setToolboxProxyModel = QgsProcessingToolboxTreeView.setToolboxProxyModel
    def __QgsProcessingToolboxTreeView_setToolboxProxyModel_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingToolboxTreeView_setToolboxProxyModel(self, arg)
    QgsProcessingToolboxTreeView.setToolboxProxyModel = _functools.update_wrapper(__QgsProcessingToolboxTreeView_setToolboxProxyModel_wrapper, QgsProcessingToolboxTreeView.setToolboxProxyModel)

    QgsProcessingToolboxTreeView.__group__ = ['processing']
except (NameError, AttributeError):
    pass
