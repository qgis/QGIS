# The following has been generated automatically from src/core/locator/qgslocator.h
try:
    QgsLocator.__attribute_docs__ = {'CORE_FILTERS': 'List of core filters (i.e. not plugin filters)', 'foundResult': 'Emitted whenever a filter encounters a matching ``result`` after the :py:func:`~QgsLocator.fetchResults` method\nis called.\n', 'searchPrepared': 'Emitted when locator has prepared the search (:py:func:`QgsLocatorFilter.prepare`)\nbefore the search is actually performed\n\n.. versionadded:: 3.16\n', 'finished': 'Emitted when locator has finished a query, either as a result\nof successful completion or early cancellation.\n'}
    QgsLocator.__signal_arguments__ = {'foundResult': ['result: QgsLocatorResult']}
    QgsLocator.__group__ = ['locator']
except (NameError, AttributeError):
    pass
