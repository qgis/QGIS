# The following has been generated automatically from src/core/pointcloud/qgspointcloudattribute.h
QgsPointCloudAttribute.Char = QgsPointCloudAttribute.DataType.Char
QgsPointCloudAttribute.UChar = QgsPointCloudAttribute.DataType.UChar
QgsPointCloudAttribute.Short = QgsPointCloudAttribute.DataType.Short
QgsPointCloudAttribute.UShort = QgsPointCloudAttribute.DataType.UShort
QgsPointCloudAttribute.Int32 = QgsPointCloudAttribute.DataType.Int32
QgsPointCloudAttribute.UInt32 = QgsPointCloudAttribute.DataType.UInt32
QgsPointCloudAttribute.Int64 = QgsPointCloudAttribute.DataType.Int64
QgsPointCloudAttribute.UInt64 = QgsPointCloudAttribute.DataType.UInt64
QgsPointCloudAttribute.Float = QgsPointCloudAttribute.DataType.Float
QgsPointCloudAttribute.Double = QgsPointCloudAttribute.DataType.Double
try:
    QgsPointCloudAttribute.isNumeric = staticmethod(QgsPointCloudAttribute.isNumeric)
    QgsPointCloudAttribute.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudAttributeCollection.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
