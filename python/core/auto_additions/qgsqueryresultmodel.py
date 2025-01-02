# The following has been generated automatically from src/core/qgsqueryresultmodel.h
try:
    QgsQueryResultModel.__attribute_docs__ = {'fetchingComplete': 'Emitted when rows have been fetched (all of them or a batch if `maxRows` was passed to :py:func:`~QgsQueryResultModel.fetchMoreRows` )\nor when the fetching has been stopped (canceled).\n\n.. seealso:: :py:func:`fetchMoreRows`\n', 'fetchMoreRows': 'Emitted when more rows are requested.\n\n:param maxRows: the number of rows that will be fetched.\n', 'fetchingStarted': 'Emitted when fetching of rows has started\n'}
    QgsQueryResultModel.__signal_arguments__ = {'fetchMoreRows': ['maxRows: int']}
except (NameError, AttributeError):
    pass
