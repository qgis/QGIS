# The following has been generated automatically from src/gui/settings/qgssettingstreemodel.h
# monkey patching scoped based enum
QgsSettingsTreeModel.Column.Name.__doc__ = "Name"
QgsSettingsTreeModel.Column.Value.__doc__ = "Value"
QgsSettingsTreeModel.Column.Description.__doc__ = "Description"
QgsSettingsTreeModel.Column.__doc__ = """Columns

* ``Name``: Name
* ``Value``: Value
* ``Description``: Description

"""
# --
try:
    QgsSettingsTreeModel.__overridden_methods__ = ['index', 'parent', 'rowCount', 'columnCount', 'data', 'headerData', 'flags', 'setData']
    QgsSettingsTreeModel.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsTreeProxyModel.__overridden_methods__ = ['filterAcceptsRow']
    QgsSettingsTreeProxyModel.__group__ = ['settings']
except (NameError, AttributeError):
    pass
