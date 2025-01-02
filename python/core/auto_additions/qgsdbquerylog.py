# The following has been generated automatically from src/core/qgsdbquerylog.h
try:
    QgsDatabaseQueryLogEntry.__attribute_docs__ = {'queryId': 'Unique query ID.\n\nThis ID will automatically be set on creation of a new QgsDatabaseQueryLogEntry object.', 'uri': 'Database URI', 'provider': 'Provider key', 'query': 'The logged database query (e.g. the SQL query)', 'startedTime': 'Time when the query started (in milliseconds since epoch).\n\nThis will be automatically recorded on creation of a new QgsDatabaseQueryLogEntry object.', 'finishedTime': 'Time when the query finished (in milliseconds since epoch), if available.', 'initiatorClass': 'The QGIS class which initiated the query.\n\nc++ code can automatically populate this through the :py:class:`QgsSetQueryLogClass` macro.', 'origin': 'Code file location for the query origin.\n\nc++ code can automatically populate this through the :py:class:`QgsSetQueryLogClass` macro.', 'fetchedRows': 'Number of fetched/affected rows.\n\n.. warning::\n\n   Not all providers support this information.', 'error': 'Error reported by the provider, normally blank', 'canceled': 'Canceled flag for user canceled queries.'}
except (NameError, AttributeError):
    pass
try:
    QgsDatabaseQueryLog.enabled = staticmethod(QgsDatabaseQueryLog.enabled)
    QgsDatabaseQueryLog.log = staticmethod(QgsDatabaseQueryLog.log)
    QgsDatabaseQueryLog.finished = staticmethod(QgsDatabaseQueryLog.finished)
except (NameError, AttributeError):
    pass
