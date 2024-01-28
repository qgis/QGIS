# The following has been generated automatically from src/core/layout/qgslayoutmanager.h
QgsLayoutManagerModel.LayoutRole = QgsLayoutManagerModel.Role.LayoutRole
QgsLayoutManagerProxyModel.FilterPrintLayouts = QgsLayoutManagerProxyModel.Filter.FilterPrintLayouts
QgsLayoutManagerProxyModel.FilterReports = QgsLayoutManagerProxyModel.Filter.FilterReports
QgsLayoutManagerProxyModel.Filters = lambda flags=0: QgsLayoutManagerProxyModel.Filter(flags)
QgsLayoutManagerProxyModel.Filters.baseClass = QgsLayoutManagerProxyModel
Filters = QgsLayoutManagerProxyModel  # dirty hack since SIP seems to introduce the flags in module
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsLayoutManagerProxyModel.Filter.__bool__ = lambda flag: _force_int(flag)
QgsLayoutManagerProxyModel.Filter.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsLayoutManagerProxyModel.Filter.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsLayoutManagerProxyModel.Filter.__or__ = lambda flag1, flag2: QgsLayoutManagerProxyModel.Filter(_force_int(flag1) | _force_int(flag2))
