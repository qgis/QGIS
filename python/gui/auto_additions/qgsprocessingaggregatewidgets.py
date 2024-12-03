# The following has been generated automatically from src/gui/processing/qgsprocessingaggregatewidgets.h
# monkey patching scoped based enum
QgsAggregateMappingModel.ColumnDataIndex.SourceExpression.__doc__ = "Expression"
QgsAggregateMappingModel.ColumnDataIndex.Aggregate.__doc__ = "Aggregate name"
QgsAggregateMappingModel.ColumnDataIndex.Delimiter.__doc__ = "Delimiter"
QgsAggregateMappingModel.ColumnDataIndex.DestinationName.__doc__ = "Destination field name"
QgsAggregateMappingModel.ColumnDataIndex.DestinationType.__doc__ = "Destination field type string"
QgsAggregateMappingModel.ColumnDataIndex.DestinationLength.__doc__ = "Destination field length"
QgsAggregateMappingModel.ColumnDataIndex.DestinationPrecision.__doc__ = "Destination field precision"
QgsAggregateMappingModel.ColumnDataIndex.__doc__ = """The ColumnDataIndex enum represents the column index for the view

* ``SourceExpression``: Expression
* ``Aggregate``: Aggregate name
* ``Delimiter``: Delimiter
* ``DestinationName``: Destination field name
* ``DestinationType``: Destination field type string
* ``DestinationLength``: Destination field length
* ``DestinationPrecision``: Destination field precision

"""
# --
QgsAggregateMappingModel.ColumnDataIndex.baseClass = QgsAggregateMappingModel
try:
    QgsAggregateMappingModel.Aggregate.__attribute_docs__ = {'source': 'The source expression used as the input for the aggregate calculation', 'aggregate': 'Aggregate name', 'delimiter': 'Delimiter string', 'field': 'The field in its current status (it might have been renamed)'}
    QgsAggregateMappingModel.Aggregate.__doc__ = """The Aggregate struct holds information about an aggregate column"""
    QgsAggregateMappingModel.Aggregate.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsAggregateMappingWidget.__attribute_docs__ = {'changed': 'Emitted when the aggregates defined in the widget are changed.\n'}
    QgsAggregateMappingWidget.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsAggregateMappingModel.__group__ = ['processing']
except (NameError, AttributeError):
    pass
