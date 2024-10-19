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
QgsPointCloudAttributeModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsPointCloudAttributeModel.FieldRoles

.. versionadded:: 3.36

* ``AttributeName``: Attribute name

  Available as ``QgsPointCloudAttributeModel.AttributeNameRole`` in older QGIS releases.

* ``AttributeIndex``: Attribute index if index corresponds to an attribute

  Available as ``QgsPointCloudAttributeModel.AttributeIndexRole`` in older QGIS releases.

* ``AttributeSize``: Attribute size

  Available as ``QgsPointCloudAttributeModel.AttributeSizeRole`` in older QGIS releases.

* ``AttributeType``: Attribute type, see QgsPointCloudAttribute.DataType

  Available as ``QgsPointCloudAttributeModel.AttributeTypeRole`` in older QGIS releases.

* ``IsEmpty``: ``True`` if the index corresponds to the empty value

  Available as ``QgsPointCloudAttributeModel.IsEmptyRole`` in older QGIS releases.

* ``IsNumeric``: ``True`` if the index corresponds to a numeric attributre

  Available as ``QgsPointCloudAttributeModel.IsNumericRole`` in older QGIS releases.


"""
# --
QgsPointCloudAttributeModel.CustomRole.baseClass = QgsPointCloudAttributeModel
QgsPointCloudAttributeProxyModel.Filters.baseClass = QgsPointCloudAttributeProxyModel
Filters = QgsPointCloudAttributeProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsPointCloudAttributeModel.attributeToolTip = staticmethod(QgsPointCloudAttributeModel.attributeToolTip)
    QgsPointCloudAttributeModel.iconForAttributeType = staticmethod(QgsPointCloudAttributeModel.iconForAttributeType)
    QgsPointCloudAttributeModel.__group__ = ['pointcloud']
except NameError:
    pass
try:
    QgsPointCloudAttributeProxyModel.__group__ = ['pointcloud']
except NameError:
    pass
