# The following has been generated automatically from src/core/raster/qgsrasterfilewritertask.h
try:
    QgsRasterFileWriterTask.__attribute_docs__ = {'writeComplete': 'Emitted when writing the layer is successfully completed. The ``outputUrl``\nparameter indicates the file path for the written file(s).\n', 'errorOccurred': 'Emitted when an error occurs which prevented the file being written (or if\nthe task is canceled). The writing ``error`` will be reported and a\n``errorMessage`` will be potentially set.\n\n.. versionadded:: 3.10\n'}
    QgsRasterFileWriterTask.__signal_arguments__ = {'writeComplete': ['outputUrl: str'], 'errorOccurred': ['error: int', 'errorMessage: str']}
    QgsRasterFileWriterTask.__group__ = ['raster']
except (NameError, AttributeError):
    pass
