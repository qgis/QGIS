# The following has been generated automatically from src/core/pointcloud/qgspointcloudattributemodel.h
QgsPointCloudAttributeModel.FieldRoles = QgsPointCloudAttributeModel.CustomRole
# monkey patching scoped based enum
QgsPointCloudAttributeModel.AttributeNameRole = QgsPointCloudAttributeModel.CustomRole.AttributeName
QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole = QgsPointCloudAttributeModel.CustomRole.AttributeName
QgsPointCloudAttributeModel.AttributeNameRole.is_monkey_patched = True
QgsPointCloudAttributeModel.AttributeNameRole.__doc__ = "Attribute name"
QgsPointCloudAttributeModel.AttributeIndexRole = QgsPointCloudAttributeModel.CustomRole.AttributeIndex
QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole = QgsPointCloudAttributeModel.CustomRole.AttributeIndex
QgsPointCloudAttributeModel.AttributeIndexRole.is_monkey_patched = True
QgsPointCloudAttributeModel.AttributeIndexRole.__doc__ = "Attribute index if index corresponds to an attribute"
QgsPointCloudAttributeModel.AttributeSizeRole = QgsPointCloudAttributeModel.CustomRole.AttributeSize
QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole = QgsPointCloudAttributeModel.CustomRole.AttributeSize
QgsPointCloudAttributeModel.AttributeSizeRole.is_monkey_patched = True
QgsPointCloudAttributeModel.AttributeSizeRole.__doc__ = "Attribute size"
QgsPointCloudAttributeModel.AttributeTypeRole = QgsPointCloudAttributeModel.CustomRole.AttributeType
QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole = QgsPointCloudAttributeModel.CustomRole.AttributeType
QgsPointCloudAttributeModel.AttributeTypeRole.is_monkey_patched = True
QgsPointCloudAttributeModel.AttributeTypeRole.__doc__ = "Attribute type, see QgsPointCloudAttribute.DataType"
QgsPointCloudAttributeModel.IsEmptyRole = QgsPointCloudAttributeModel.CustomRole.IsEmpty
QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole = QgsPointCloudAttributeModel.CustomRole.IsEmpty
QgsPointCloudAttributeModel.IsEmptyRole.is_monkey_patched = True
QgsPointCloudAttributeModel.IsEmptyRole.__doc__ = "``True`` if the index corresponds to the empty value"
QgsPointCloudAttributeModel.IsNumericRole = QgsPointCloudAttributeModel.CustomRole.IsNumeric
QgsPointCloudAttributeModel.FieldRoles.IsNumericRole = QgsPointCloudAttributeModel.CustomRole.IsNumeric
QgsPointCloudAttributeModel.IsNumericRole.is_monkey_patched = True
QgsPointCloudAttributeModel.IsNumericRole.__doc__ = "``True`` if the index corresponds to a numeric attributre"
QgsPointCloudAttributeModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsPointCloudAttributeModel.FieldRoles\n\n.. versionadded:: 3.36\n\n" + '* ``AttributeNameRole``: ' + QgsPointCloudAttributeModel.CustomRole.AttributeName.__doc__ + '\n' + '* ``AttributeIndexRole``: ' + QgsPointCloudAttributeModel.CustomRole.AttributeIndex.__doc__ + '\n' + '* ``AttributeSizeRole``: ' + QgsPointCloudAttributeModel.CustomRole.AttributeSize.__doc__ + '\n' + '* ``AttributeTypeRole``: ' + QgsPointCloudAttributeModel.CustomRole.AttributeType.__doc__ + '\n' + '* ``IsEmptyRole``: ' + QgsPointCloudAttributeModel.CustomRole.IsEmpty.__doc__ + '\n' + '* ``IsNumericRole``: ' + QgsPointCloudAttributeModel.CustomRole.IsNumeric.__doc__
# --
QgsPointCloudAttributeModel.CustomRole.baseClass = QgsPointCloudAttributeModel
QgsPointCloudAttributeProxyModel.Filters.baseClass = QgsPointCloudAttributeProxyModel
Filters = QgsPointCloudAttributeProxyModel  # dirty hack since SIP seems to introduce the flags in module
QgsPointCloudAttributeModel.attributeToolTip = staticmethod(QgsPointCloudAttributeModel.attributeToolTip)
QgsPointCloudAttributeModel.iconForAttributeType = staticmethod(QgsPointCloudAttributeModel.iconForAttributeType)
try:
    QgsPointCloudAttributeModel.__group__ = ['pointcloud']
except NameError:
    pass
try:
    QgsPointCloudAttributeProxyModel.__group__ = ['pointcloud']
except NameError:
    pass
