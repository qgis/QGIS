# The following has been generated automatically from src/gui/attributetable/qgsattributetablefiltermodel.h
QgsAttributeTableFilterModel.ShowAll = QgsAttributeTableFilterModel.FilterMode.ShowAll
QgsAttributeTableFilterModel.ShowSelected = QgsAttributeTableFilterModel.FilterMode.ShowSelected
QgsAttributeTableFilterModel.ShowVisible = QgsAttributeTableFilterModel.FilterMode.ShowVisible
QgsAttributeTableFilterModel.ShowFilteredList = QgsAttributeTableFilterModel.FilterMode.ShowFilteredList
QgsAttributeTableFilterModel.ShowEdited = QgsAttributeTableFilterModel.FilterMode.ShowEdited
QgsAttributeTableFilterModel.ShowInvalid = QgsAttributeTableFilterModel.FilterMode.ShowInvalid
QgsAttributeTableFilterModel.FilterMode.baseClass = QgsAttributeTableFilterModel
QgsAttributeTableFilterModel.ColumnTypeField = QgsAttributeTableFilterModel.ColumnType.ColumnTypeField
QgsAttributeTableFilterModel.ColumnTypeActionButton = QgsAttributeTableFilterModel.ColumnType.ColumnTypeActionButton
QgsAttributeTableFilterModel.ColumnType.baseClass = QgsAttributeTableFilterModel
QgsAttributeTableFilterModel.Role = QgsAttributeTableFilterModel.CustomRole
# monkey patching scoped based enum
QgsAttributeTableFilterModel.TypeRole = QgsAttributeTableFilterModel.CustomRole.Type
QgsAttributeTableFilterModel.Role.TypeRole = QgsAttributeTableFilterModel.CustomRole.Type
QgsAttributeTableFilterModel.TypeRole.is_monkey_patched = True
QgsAttributeTableFilterModel.TypeRole.__doc__ = ""
QgsAttributeTableFilterModel.CustomRole.__doc__ = "The additional roles defined by this filter model.\nThe values of these roles start just after the roles defined by\n:py:class:`QgsAttributeTableModel` so they do not conflict.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsAttributeTableFilterModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``TypeRole``: ' + QgsAttributeTableFilterModel.CustomRole.Type.__doc__
# --
QgsAttributeTableFilterModel.CustomRole.baseClass = QgsAttributeTableFilterModel
