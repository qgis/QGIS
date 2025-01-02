# The following has been generated automatically from src/core/qgspointlocator.h
try:
    QgsPointLocator.__attribute_docs__ = {'initFinished': 'Emitted whenever index has been built and initialization is finished\n\n:param ok: ``False`` if the creation of index has been prematurely stopped due to the limit of\n           features, otherwise ``True``\n'}
    QgsPointLocator.__signal_arguments__ = {'initFinished': ['ok: bool']}
except (NameError, AttributeError):
    pass
try:
    QgsPointLocator.MatchFilter.__doc__ = """Interface that allows rejection of some matches in intersection queries
(e.g. a match can only belong to a particular feature / match must not be a particular point).
Implement the interface and pass its instance to QgsPointLocator or QgsSnappingUtils methods."""
except (NameError, AttributeError):
    pass
