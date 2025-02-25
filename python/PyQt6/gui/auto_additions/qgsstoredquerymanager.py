# The following has been generated automatically from src/gui/qgsstoredquerymanager.h
try:
    QgsStoredQueryManager.QueryDetails.__attribute_docs__ = {'name': 'Name of the query.', 'definition': 'Query definition.', 'backend': 'Storage backend.'}
except (NameError, AttributeError):
    pass
try:
    QgsStoredQueryManager.__attribute_docs__ = {'queryAdded': 'Emitted when a query is added to the manager.\n', 'queryChanged': 'Emitted when an existing query is changed in the manager.\n', 'queryRemoved': 'Emitted when a query is removed from the manager.\n'}
    QgsStoredQueryManager.__signal_arguments__ = {'queryAdded': ['name: str', 'backend: Qgis.QueryStorageBackend'], 'queryChanged': ['name: str', 'backend: Qgis.QueryStorageBackend'], 'queryRemoved': ['name: str', 'backend: Qgis.QueryStorageBackend']}
except (NameError, AttributeError):
    pass
