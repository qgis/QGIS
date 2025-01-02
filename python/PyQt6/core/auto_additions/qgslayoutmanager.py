# The following has been generated automatically from src/core/layout/qgslayoutmanager.h
QgsLayoutManagerModel.Role = QgsLayoutManagerModel.CustomRole
# monkey patching scoped based enum
QgsLayoutManagerModel.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.Role.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.LayoutRole.is_monkey_patched = True
QgsLayoutManagerModel.LayoutRole.__doc__ = "Layout object"
QgsLayoutManagerModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsLayoutManagerModel.Role

.. versionadded:: 3.36

* ``Layout``: Layout object

  Available as ``QgsLayoutManagerModel.LayoutRole`` in older QGIS releases.


"""
# --
QgsLayoutManagerModel.CustomRole.baseClass = QgsLayoutManagerModel
QgsLayoutManagerProxyModel.FilterPrintLayouts = QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts
QgsLayoutManagerProxyModel.FilterReports = QgsLayoutManagerProxyModel.Filter.FilterReports
QgsLayoutManagerProxyModel.Filters = lambda flags=0: QgsLayoutManagerProxyModel.Filter(flags)
QgsLayoutManagerProxyModel.Filters.baseClass = QgsLayoutManagerProxyModel
Filters = QgsLayoutManagerProxyModel  # dirty hack since SIP seems to introduce the flags in module
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsLayoutManagerProxyModel.Filter.__bool__ = lambda flag: bool(_force_int(flag))
QgsLayoutManagerProxyModel.Filter.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsLayoutManagerProxyModel.Filter.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsLayoutManagerProxyModel.Filter.__or__ = lambda flag1, flag2: QgsLayoutManagerProxyModel.Filter(_force_int(flag1) | _force_int(flag2))
try:
    QgsLayoutManager.__attribute_docs__ = {'layoutAboutToBeAdded': 'Emitted when a layout is about to be added to the manager\n', 'layoutAdded': 'Emitted when a layout has been added to the manager\n', 'layoutRemoved': 'Emitted when a layout was removed from the manager\n', 'layoutAboutToBeRemoved': 'Emitted when a layout is about to be removed from the manager\n', 'layoutRenamed': 'Emitted when a layout is renamed\n'}
    QgsLayoutManager.__signal_arguments__ = {'layoutAboutToBeAdded': ['name: str'], 'layoutAdded': ['name: str'], 'layoutRemoved': ['name: str'], 'layoutAboutToBeRemoved': ['name: str'], 'layoutRenamed': ['layout: QgsMasterLayoutInterface', 'newName: str']}
    QgsLayoutManager.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutManagerModel.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutManagerProxyModel.__group__ = ['layout']
except (NameError, AttributeError):
    pass
