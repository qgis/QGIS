# The following has been generated automatically from src/gui/processing/qgsprocessinghistoryprovider.h
try:
    QgsProcessingHistoryProvider.__attribute_docs__ = {'executePython': 'Emitted when the provider needs to execute python ``commands`` in the\nProcessing context.\n\n.. versionadded:: 3.32\n', 'createTest': 'Emitted when the provider needs to create a Processing test with the\ngiven python ``command``.\n\n.. versionadded:: 3.32\n'}
    QgsProcessingHistoryProvider.__overridden_methods__ = ['id', 'createNodeForEntry', 'updateNodeForEntry']
    QgsProcessingHistoryProvider.__signal_arguments__ = {'executePython': ['commands: str'], 'createTest': ['command: str']}
    QgsProcessingHistoryProvider.__group__ = ['processing']
except (NameError, AttributeError):
    pass
