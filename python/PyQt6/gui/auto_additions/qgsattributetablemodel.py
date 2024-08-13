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
QgsAttributeTableModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsAttributeTableModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``FeatureIdRole``: ' + QgsAttributeTableModel.CustomRole.FeatureId.__doc__ + '\n' + '* ``FieldIndexRole``: ' + QgsAttributeTableModel.CustomRole.FieldIndex.__doc__ + '\n' + '* ``UserRole``: ' + QgsAttributeTableModel.CustomRole.User.__doc__ + '\n' + '* ``SortRole``: ' + QgsAttributeTableModel.CustomRole.Sort.__doc__
# --
QgsAttributeTableModel.CustomRole.baseClass = QgsAttributeTableModel
QgsAttributeTableModel.__attribute_docs__ = {'modelChanged': 'Model has been changed\n'}
