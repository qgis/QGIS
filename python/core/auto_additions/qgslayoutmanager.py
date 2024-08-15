# The following has been generated automatically from src/core/layout/qgslayoutmanager.h
QgsLayoutManagerModel.Role = QgsLayoutManagerModel.CustomRole
# monkey patching scoped based enum
QgsLayoutManagerModel.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.Role.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.LayoutRole.is_monkey_patched = True
QgsLayoutManagerModel.LayoutRole.__doc__ = "Layout object"
QgsLayoutManagerModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsLayoutManagerModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``LayoutRole``: ' + QgsLayoutManagerModel.CustomRole.Layout.__doc__
# --
QgsLayoutManagerModel.CustomRole.baseClass = QgsLayoutManagerModel
QgsLayoutManagerProxyModel.Filters.baseClass = QgsLayoutManagerProxyModel
Filters = QgsLayoutManagerProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsLayoutManager.__attribute_docs__ = {'layoutAboutToBeAdded': 'Emitted when a layout is about to be added to the manager\n', 'layoutAdded': 'Emitted when a layout has been added to the manager\n', 'layoutRemoved': 'Emitted when a layout was removed from the manager\n', 'layoutAboutToBeRemoved': 'Emitted when a layout is about to be removed from the manager\n', 'layoutRenamed': 'Emitted when a layout is renamed\n'}
except NameError:
    pass
try:
    QgsLayoutManager.__group__ = ['layout']
except NameError:
    pass
try:
    QgsLayoutManagerModel.__group__ = ['layout']
except NameError:
    pass
try:
    QgsLayoutManagerProxyModel.__group__ = ['layout']
except NameError:
    pass
