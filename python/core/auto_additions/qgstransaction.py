# The following has been generated automatically from src/core/qgstransaction.h
try:
    QgsTransaction.__attribute_docs__ = {'afterRollback': 'Emitted after a rollback\n', 'dirtied': 'Emitted if a sql query is executed and the underlying data is modified\n'}
    QgsTransaction.create = staticmethod(QgsTransaction.create)
    QgsTransaction.supportsTransaction = staticmethod(QgsTransaction.supportsTransaction)
    QgsTransaction.__signal_arguments__ = {'dirtied': ['sql: str', 'name: str']}
except (NameError, AttributeError):
    pass
