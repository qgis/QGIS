# The following has been generated automatically from src/core/project/qgsprojectstylesettings.h
QgsProjectStyleDatabaseModel.Role = QgsProjectStyleDatabaseModel.CustomRole
# monkey patching scoped based enum
QgsProjectStyleDatabaseModel.StyleRole = QgsProjectStyleDatabaseModel.CustomRole.Style
QgsProjectStyleDatabaseModel.Role.StyleRole = QgsProjectStyleDatabaseModel.CustomRole.Style
QgsProjectStyleDatabaseModel.StyleRole.is_monkey_patched = True
QgsProjectStyleDatabaseModel.StyleRole.__doc__ = "Style object"
QgsProjectStyleDatabaseModel.PathRole = QgsProjectStyleDatabaseModel.CustomRole.Path
QgsProjectStyleDatabaseModel.Role.PathRole = QgsProjectStyleDatabaseModel.CustomRole.Path
QgsProjectStyleDatabaseModel.PathRole.is_monkey_patched = True
QgsProjectStyleDatabaseModel.PathRole.__doc__ = "Style path"
QgsProjectStyleDatabaseModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsProjectStyleDatabaseModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``StyleRole``: ' + QgsProjectStyleDatabaseModel.CustomRole.Style.__doc__ + '\n' + '* ``PathRole``: ' + QgsProjectStyleDatabaseModel.CustomRole.Path.__doc__
# --
QgsProjectStyleDatabaseModel.CustomRole.baseClass = QgsProjectStyleDatabaseModel
# monkey patching scoped based enum
QgsProjectStyleDatabaseProxyModel.Filter.FilterHideReadOnly.__doc__ = "Hide read-only style databases"
QgsProjectStyleDatabaseProxyModel.Filter.__doc__ = "Available filter flags for filtering the model\n\n" + '* ``FilterHideReadOnly``: ' + QgsProjectStyleDatabaseProxyModel.Filter.FilterHideReadOnly.__doc__
# --
QgsProjectStyleDatabaseProxyModel.Filter.baseClass = QgsProjectStyleDatabaseProxyModel
QgsProjectStyleDatabaseProxyModel.Filters.baseClass = QgsProjectStyleDatabaseProxyModel
Filters = QgsProjectStyleDatabaseProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsProjectStyleSettings.__attribute_docs__ = {'styleDatabasesChanged': 'Emitted whenever the set of style databases associated with the project is changed.\n'}
except NameError:
    pass
