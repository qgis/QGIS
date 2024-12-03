# The following has been generated automatically from src/gui/qgsfieldmappingmodel.h
# monkey patching scoped based enum
QgsFieldMappingModel.ColumnDataIndex.SourceExpression.__doc__ = "Expression"
QgsFieldMappingModel.ColumnDataIndex.DestinationName.__doc__ = "Destination field name"
QgsFieldMappingModel.ColumnDataIndex.DestinationType.__doc__ = "Destination field type string"
QgsFieldMappingModel.ColumnDataIndex.DestinationLength.__doc__ = "Destination field length"
QgsFieldMappingModel.ColumnDataIndex.DestinationPrecision.__doc__ = "Destination field precision"
QgsFieldMappingModel.ColumnDataIndex.DestinationConstraints.__doc__ = "Destination field constraints"
QgsFieldMappingModel.ColumnDataIndex.DestinationAlias.__doc__ = "Destination alias"
QgsFieldMappingModel.ColumnDataIndex.DestinationComment.__doc__ = "Destination comment"
QgsFieldMappingModel.ColumnDataIndex.__doc__ = """The ColumnDataIndex enum represents the column index for the view

* ``SourceExpression``: Expression
* ``DestinationName``: Destination field name
* ``DestinationType``: Destination field type string
* ``DestinationLength``: Destination field length
* ``DestinationPrecision``: Destination field precision
* ``DestinationConstraints``: Destination field constraints
* ``DestinationAlias``: Destination alias
* ``DestinationComment``: Destination comment

"""
# --
QgsFieldMappingModel.ColumnDataIndex.baseClass = QgsFieldMappingModel
try:
    QgsFieldMappingModel.Field.__attribute_docs__ = {'originalName': 'The original name of the field', 'field': 'The field in its current status (it might have been renamed)', 'expression': 'The expression for the mapped field from the source fields'}
    QgsFieldMappingModel.Field.__doc__ = """The Field struct holds information about a mapped field"""
except (NameError, AttributeError):
    pass
