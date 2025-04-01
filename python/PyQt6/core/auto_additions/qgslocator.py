# The following has been generated automatically from src/core/locator/qgslocator.h
try:
    QgsLocator.__attribute_docs__ = {'CORE_FILTERS': 'List of core filters (i.e. not plugin filters)', 'foundResult': 'Emitted whenever a filter encounters a matching ``result`` after the\n:py:func:`~QgsLocator.fetchResults` method is called.\n', 'searchPrepared': 'Emitted when locator has prepared the search\n(:py:func:`QgsLocatorFilter.prepare`) before the search is actually\nperformed\n\n.. versionadded:: 3.16\n', 'finished': 'Emitted when locator has finished a query, either as a result of\nsuccessful completion or early cancellation.\n'}
    QgsLocator.__annotations__ = {'CORE_FILTERS': 'List[str]'}
    QgsLocator.__signal_arguments__ = {'foundResult': ['result: QgsLocatorResult']}
    QgsLocator.__group__ = ['locator']
except (NameError, AttributeError):
    pass
