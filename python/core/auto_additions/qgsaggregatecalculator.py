# The following has been generated automatically from src/core/qgsaggregatecalculator.h
try:
    QgsAggregateCalculator.AggregateInfo.__attribute_docs__ = {'function': 'The expression function', 'name': 'A translated, human readable name', 'supportedTypes': 'This aggregate function can only be used with these datatypes'}
    QgsAggregateCalculator.AggregateInfo.__doc__ = """Structured information about the available aggregates."""
except (NameError, AttributeError):
    pass
try:
    QgsAggregateCalculator.AggregateParameters.__attribute_docs__ = {'filter': 'Optional filter for calculating aggregate over a subset of features, or an\nempty string to use all features.\n\n.. seealso:: :py:func:`QgsAggregateCalculator.setFilter`\n\n.. seealso:: :py:func:`QgsAggregateCalculator.filter`', 'delimiter': 'Delimiter to use for joining values with the StringConcatenate aggregate.\n\n.. seealso:: :py:func:`QgsAggregateCalculator.setDelimiter`\n\n.. seealso:: :py:func:`QgsAggregateCalculator.delimiter`', 'orderBy': 'Optional order by clauses.\n\n.. versionadded:: 3.8'}
    QgsAggregateCalculator.AggregateParameters.__doc__ = """A bundle of parameters controlling aggregate calculation"""
except (NameError, AttributeError):
    pass
try:
    QgsAggregateCalculator.stringToAggregate = staticmethod(QgsAggregateCalculator.stringToAggregate)
    QgsAggregateCalculator.displayName = staticmethod(QgsAggregateCalculator.displayName)
    QgsAggregateCalculator.aggregates = staticmethod(QgsAggregateCalculator.aggregates)
except (NameError, AttributeError):
    pass
