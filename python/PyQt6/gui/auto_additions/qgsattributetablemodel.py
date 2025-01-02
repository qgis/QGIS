# The following has been generated automatically from src/gui/attributetable/qgsattributetablemodel.h
QgsAttributeTableModel.Role = QgsAttributeTableModel.CustomRole
# monkey patching scoped based enum
QgsAttributeTableModel.FeatureIdRole = QgsAttributeTableModel.CustomRole.FeatureId
QgsAttributeTableModel.Role.FeatureIdRole = QgsAttributeTableModel.CustomRole.FeatureId
QgsAttributeTableModel.FeatureIdRole.is_monkey_patched = True
QgsAttributeTableModel.FeatureIdRole.__doc__ = "Get the feature id of the feature in this row"
QgsAttributeTableModel.FieldIndexRole = QgsAttributeTableModel.CustomRole.FieldIndex
QgsAttributeTableModel.Role.FieldIndexRole = QgsAttributeTableModel.CustomRole.FieldIndex
QgsAttributeTableModel.FieldIndexRole.is_monkey_patched = True
QgsAttributeTableModel.FieldIndexRole.__doc__ = "Get the field index of this column"
QgsAttributeTableModel.UserRole = QgsAttributeTableModel.CustomRole.User
QgsAttributeTableModel.Role.UserRole = QgsAttributeTableModel.CustomRole.User
QgsAttributeTableModel.UserRole.is_monkey_patched = True
QgsAttributeTableModel.UserRole.__doc__ = "Start further roles starting from this role"
QgsAttributeTableModel.SortRole = QgsAttributeTableModel.CustomRole.Sort
QgsAttributeTableModel.Role.SortRole = QgsAttributeTableModel.CustomRole.Sort
QgsAttributeTableModel.SortRole.is_monkey_patched = True
QgsAttributeTableModel.SortRole.__doc__ = "Role used for sorting start here"
QgsAttributeTableModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsAttributeTableModel.Role

.. versionadded:: 3.36

* ``FeatureId``: Get the feature id of the feature in this row

  Available as ``QgsAttributeTableModel.FeatureIdRole`` in older QGIS releases.

* ``FieldIndex``: Get the field index of this column

  Available as ``QgsAttributeTableModel.FieldIndexRole`` in older QGIS releases.

* ``User``: Start further roles starting from this role

  Available as ``QgsAttributeTableModel.UserRole`` in older QGIS releases.

* ``Sort``: Role used for sorting start here

  Available as ``QgsAttributeTableModel.SortRole`` in older QGIS releases.


"""
# --
QgsAttributeTableModel.CustomRole.baseClass = QgsAttributeTableModel
try:
    QgsAttributeTableModel.__attribute_docs__ = {'modelChanged': 'Emitted when the model has been changed.\n', 'finished': 'Emitted when the model has completely loaded all features.\n'}
    QgsAttributeTableModel.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
