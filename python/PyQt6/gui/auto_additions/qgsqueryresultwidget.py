# The following has been generated automatically from src/gui/qgsqueryresultwidget.h
# monkey patching scoped based enum
QgsQueryResultWidget.QueryWidgetMode.SqlQueryMode.__doc__ = "Defaults widget mode for SQL execution and SQL query layer creation."
QgsQueryResultWidget.QueryWidgetMode.QueryLayerUpdateMode.__doc__ = "SQL query layer update mode: the create SQL layer button is renamed to 'Update' and the SQL layer creation group box is expanded."
QgsQueryResultWidget.QueryWidgetMode.__doc__ = "The QueryWidgetMode enum represents various modes for the widget appearance.\n\n" + '* ``SqlQueryMode``: ' + QgsQueryResultWidget.QueryWidgetMode.SqlQueryMode.__doc__ + '\n' + '* ``QueryLayerUpdateMode``: ' + QgsQueryResultWidget.QueryWidgetMode.QueryLayerUpdateMode.__doc__
# --
QgsQueryResultWidget.QueryWidgetMode.baseClass = QgsQueryResultWidget
try:
    QgsQueryResultWidget.__attribute_docs__ = {'createSqlVectorLayer': 'Emitted when a new vector SQL (query) layer must be created.\n\n:param providerKey: name of the data provider\n:param connectionUri: the connection URI as returned by :py:func:`QgsAbstractProviderConnection.uri()`\n:param options:\n', 'firstResultBatchFetched': 'Emitted when the first batch of results has been fetched.\n\n.. note::\n\n   If the query returns no results this signal is not emitted.\n'}
except NameError:
    pass
