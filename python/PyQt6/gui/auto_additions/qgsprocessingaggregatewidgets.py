# The following has been generated automatically from src/gui/processing/qgsprocessingaggregatewidgets.h
# monkey patching scoped based enum
QgsAggregateMappingModel.ColumnDataIndex.SourceExpression.__doc__ = "Expression"
QgsAggregateMappingModel.ColumnDataIndex.Aggregate.__doc__ = "Aggregate name"
QgsAggregateMappingModel.ColumnDataIndex.Delimiter.__doc__ = "Delimiter"
QgsAggregateMappingModel.ColumnDataIndex.DestinationName.__doc__ = "Destination field name"
QgsAggregateMappingModel.ColumnDataIndex.DestinationType.__doc__ = "Destination field type string"
QgsAggregateMappingModel.ColumnDataIndex.DestinationLength.__doc__ = "Destination field length"
QgsAggregateMappingModel.ColumnDataIndex.DestinationPrecision.__doc__ = "Destination field precision"
QgsAggregateMappingModel.ColumnDataIndex.__doc__ = "The ColumnDataIndex enum represents the column index for the view\n\n" + '* ``SourceExpression``: ' + QgsAggregateMappingModel.ColumnDataIndex.SourceExpression.__doc__ + '\n' + '* ``Aggregate``: ' + QgsAggregateMappingModel.ColumnDataIndex.Aggregate.__doc__ + '\n' + '* ``Delimiter``: ' + QgsAggregateMappingModel.ColumnDataIndex.Delimiter.__doc__ + '\n' + '* ``DestinationName``: ' + QgsAggregateMappingModel.ColumnDataIndex.DestinationName.__doc__ + '\n' + '* ``DestinationType``: ' + QgsAggregateMappingModel.ColumnDataIndex.DestinationType.__doc__ + '\n' + '* ``DestinationLength``: ' + QgsAggregateMappingModel.ColumnDataIndex.DestinationLength.__doc__ + '\n' + '* ``DestinationPrecision``: ' + QgsAggregateMappingModel.ColumnDataIndex.DestinationPrecision.__doc__
# --
QgsAggregateMappingModel.ColumnDataIndex.baseClass = QgsAggregateMappingModel
try:
    QgsAggregateMappingModel.__attribute_docs__ = {'source': 'The source expression used as the input for the aggregate calculation', 'aggregate': 'Aggregate name', 'delimiter': 'Delimiter string', 'field': 'The field in its current status (it might have been renamed)'}
except NameError:
    pass
try:
    QgsAggregateMappingWidget.__attribute_docs__ = {'changed': 'Emitted when the aggregates defined in the widget are changed.\n'}
except NameError:
    pass
