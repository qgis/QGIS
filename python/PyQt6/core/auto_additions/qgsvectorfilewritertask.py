# The following has been generated automatically from src/core/qgsvectorfilewritertask.h
try:
    QgsVectorFileWriterTask.__attribute_docs__ = {'writeComplete': 'Emitted when writing the layer is successfully completed. The\n``newFilename`` parameter indicates the file path for the written file.\n\n.. note::\n\n   this signal is deprecated in favor of :py:func:`~QgsVectorFileWriterTask.completed`.\n', 'errorOccurred': 'Emitted when an error occurs which prevented the file being written (or\nif the task is canceled). The writing ``error`` and ``errorMessage``\nwill be reported.\n'}
    QgsVectorFileWriterTask.__signal_arguments__ = {'writeComplete': ['newFilename: str'], 'errorOccurred': ['error: int', 'errorMessage: str']}
except (NameError, AttributeError):
    pass
