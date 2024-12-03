# The following has been generated automatically from src/gui/history/qgshistoryproviderregistry.h
try:
    QgsHistoryProviderRegistry.HistoryEntryOptions.__attribute_docs__ = {'storageBackends': 'Target storage backends'}
    QgsHistoryProviderRegistry.HistoryEntryOptions.__group__ = ['history']
except (NameError, AttributeError):
    pass
try:
    QgsHistoryProviderRegistry.__attribute_docs__ = {'entryAdded': 'Emitted when an ``entry`` is added.\n\n.. versionadded:: 3.32\n', 'entryUpdated': 'Emitted when an ``entry`` is updated.\n\n.. versionadded:: 3.32\n', 'historyCleared': 'Emitted when the history is cleared for a ``backend``.\n\nIf ``providerId`` is non-empty then the history has only been cleared for the\nspecified provider.\n\n.. versionadded:: 3.32\n'}
    QgsHistoryProviderRegistry.userHistoryDbPath = staticmethod(QgsHistoryProviderRegistry.userHistoryDbPath)
    QgsHistoryProviderRegistry.__signal_arguments__ = {'entryAdded': ['id: int', 'entry: QgsHistoryEntry', 'backend: Qgis.HistoryProviderBackend'], 'entryUpdated': ['id: int', 'entry: Dict[str, object]', 'backend: Qgis.HistoryProviderBackend'], 'historyCleared': ['backend: Qgis.HistoryProviderBackend', 'providerId: str']}
    QgsHistoryProviderRegistry.__group__ = ['history']
except (NameError, AttributeError):
    pass
