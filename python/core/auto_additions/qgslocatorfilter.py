# The following has been generated automatically from src/core/locator/qgslocatorfilter.h
QgsLocatorFilter.Priority.baseClass = QgsLocatorFilter
QgsLocatorFilter.Flags.baseClass = QgsLocatorFilter
Flags = QgsLocatorFilter  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsLocatorResult.__attribute_docs__ = {'filter': 'Filter from which the result was obtained. This is automatically set.', 'displayString': 'String displayed for result.', 'description': 'Descriptive text for result.', 'icon': 'Icon for result.', 'score': 'Match score, from 0 - 1, where 1 represents a perfect match.', 'group': 'Group the results by categories\nIf left as empty string, this means that results are all shown without being grouped.\nIf a group is given, the results will be grouped by ``group`` under a header.\n\n.. note::\n\n   This should be translated.\n\n.. versionadded:: 3.2', 'groupScore': 'Specifies the score of the group to allow ordering.\nScore must be positive, higher scores are shown first.\nIf the scores are left to 0 or are identical,\nthe sorting of groups is made alphabetically.\n\n.. versionadded:: 3.40', 'actions': 'Additional actions to be used in a locator widget\nfor the given result. They could be displayed in\na context menu.\n\n.. versionadded:: 3.6'}
    QgsLocatorResult.__group__ = ['locator']
except (NameError, AttributeError):
    pass
try:
    QgsLocatorFilter.__attribute_docs__ = {'finished': 'Emitted when the filter finishes fetching results.\n', 'resultFetched': 'Should be emitted by filters whenever they encounter a matching result\nduring within their :py:func:`~QgsLocatorFilter.fetchResults` implementation.\n'}
    QgsLocatorFilter.stringMatches = staticmethod(QgsLocatorFilter.stringMatches)
    QgsLocatorFilter.fuzzyScore = staticmethod(QgsLocatorFilter.fuzzyScore)
    QgsLocatorFilter.__signal_arguments__ = {'resultFetched': ['result: QgsLocatorResult']}
    QgsLocatorFilter.__group__ = ['locator']
except (NameError, AttributeError):
    pass
try:
    QgsLocatorResult.ResultAction.__doc__ = """The ResultAction stores basic information for additional
actions to be used in a locator widget for the result.
They could be used in a context menu for instance.

.. versionadded:: 3.6"""
    QgsLocatorResult.ResultAction.__group__ = ['locator']
except (NameError, AttributeError):
    pass
