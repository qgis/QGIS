# The following has been generated automatically from src/core/project/qgsprojectstylesettings.h
QgsProjectStyleDatabaseModel.StyleRole = QgsProjectStyleDatabaseModel.Role.StyleRole
QgsProjectStyleDatabaseModel.PathRole = QgsProjectStyleDatabaseModel.Role.PathRole
# monkey patching scoped based enum
QgsProjectStyleDatabaseProxyModel.Filter.FilterHideReadOnly.__doc__ = "Hide read-only style databases"
QgsProjectStyleDatabaseProxyModel.Filter.__doc__ = "Available filter flags for filtering the model\n\n" + '* ``FilterHideReadOnly``: ' + QgsProjectStyleDatabaseProxyModel.Filter.FilterHideReadOnly.__doc__
# --
QgsProjectStyleDatabaseProxyModel.Filter.baseClass = QgsProjectStyleDatabaseProxyModel
QgsProjectStyleDatabaseProxyModel.Filters = lambda flags=0: QgsProjectStyleDatabaseProxyModel.Filter(flags)
QgsProjectStyleDatabaseProxyModel.Filters.baseClass = QgsProjectStyleDatabaseProxyModel
Filters = QgsProjectStyleDatabaseProxyModel  # dirty hack since SIP seems to introduce the flags in module
