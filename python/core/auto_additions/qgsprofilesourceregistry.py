# The following has been generated automatically from src/core/elevation/qgsprofilesourceregistry.h
try:
    QgsProfileSourceRegistry.__attribute_docs__ = {'profileSourceRegistered': 'Signal emitted once a profile source is registered.\n\n:param sourceId: Unique identifier of the profile source that has been\n                 registered.\n:param sourceName: Name of the profile source that has been registered.\n\n.. versionadded:: 4.0\n', 'profileSourceUnregistered': 'Signal emitted once a profile source is unregistered.\n\n:param sourceId: Unique identifier of the profile source that has been\n                 unregistered.\n\n.. versionadded:: 4.0\n'}
    QgsProfileSourceRegistry.__signal_arguments__ = {'profileSourceRegistered': ['sourceId: str', 'sourceName: str'], 'profileSourceUnregistered': ['sourceId: str']}
    QgsProfileSourceRegistry.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
