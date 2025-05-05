# The following has been generated automatically from src/gui/layertree/qgslayertreeview.h
try:
    QgsLayerTreeView.__attribute_docs__ = {'currentLayerChanged': 'Emitted when a current layer is changed\n', 'datasetsDropped': 'Emitted when datasets are dropped onto the layer tree view\n', 'contextMenuAboutToShow': 'Emitted when the context menu is about to show.\n\nAllows customization of the menu.\n\n.. versionadded:: 3.32\n'}
    QgsLayerTreeView.__overridden_methods__ = ['setModel', 'contextMenuEvent', 'mouseDoubleClickEvent', 'mouseReleaseEvent', 'keyPressEvent', 'dragEnterEvent', 'dragMoveEvent', 'dropEvent', 'resizeEvent']
    QgsLayerTreeView.__signal_arguments__ = {'currentLayerChanged': ['layer: QgsMapLayer'], 'datasetsDropped': ['event: QDropEvent'], 'contextMenuAboutToShow': ['menu: QMenu']}
    import functools as _functools
    __wrapped_QgsLayerTreeView_setMenuProvider = QgsLayerTreeView.setMenuProvider
    def __QgsLayerTreeView_setMenuProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayerTreeView_setMenuProvider(self, arg)
    QgsLayerTreeView.setMenuProvider = _functools.update_wrapper(__QgsLayerTreeView_setMenuProvider_wrapper, QgsLayerTreeView.setMenuProvider)

    QgsLayerTreeView.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsLayerTreeViewMenuProvider.__abstract_methods__ = ['createContextMenu']
    QgsLayerTreeViewMenuProvider.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsLayerTreeProxyModel.__overridden_methods__ = ['filterAcceptsRow']
    QgsLayerTreeProxyModel.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
