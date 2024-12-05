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
QgsAttributeTableFilterModel.CustomRole.__doc__ = """The additional roles defined by this filter model.
The values of these roles start just after the roles defined by
:py:class:`QgsAttributeTableModel` so they do not conflict.

.. note::

   Prior to QGIS 3.36 this was available as QgsAttributeTableFilterModel.Role

.. versionadded:: 3.36

* ``Type``: 

  Available as ``QgsAttributeTableFilterModel.TypeRole`` in older QGIS releases.


"""
# --
QgsAttributeTableFilterModel.CustomRole.baseClass = QgsAttributeTableFilterModel
try:
    QgsAttributeTableFilterModel.__attribute_docs__ = {'sortColumnChanged': 'Emitted whenever the sort column is changed\n\n:param column: The sort column\n:param order: The sort order\n', 'featuresFiltered': 'Emitted when the filtering of the features has been done\n', 'visibleReloaded': 'Emitted when the the visible features on extend are reloaded (the list is created)\n', 'filterError': 'Emitted when an error occurred while filtering features\n\n.. versionadded:: 3.18\n'}
    QgsAttributeTableFilterModel.__signal_arguments__ = {'sortColumnChanged': ['column: int', 'order: Qt.SortOrder'], 'filterError': ['errorMessage: str']}
    QgsAttributeTableFilterModel.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
