# The following has been generated automatically from src/core/processing/qgsprocessingregistry.h
try:
    QgsProcessingAlgorithmInformation.__attribute_docs__ = {'displayName': 'Algorithm display name', 'icon': 'Algorithm icon'}
    QgsProcessingAlgorithmInformation.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingRegistry.__attribute_docs__ = {'providerAdded': 'Emitted when a provider has been added to the registry.\n', 'providerRemoved': 'Emitted when a provider is removed from the registry\n', 'parameterTypeAdded': 'Emitted when a new parameter type has been added to the registry.\n\n.. versionadded:: 3.2\n', 'parameterTypeRemoved': 'Emitted when a parameter type has been removed from the\nregistry and is about to be deleted.\n\n.. versionadded:: 3.2\n'}
    QgsProcessingRegistry.__signal_arguments__ = {'providerAdded': ['id: str'], 'providerRemoved': ['id: str'], 'parameterTypeAdded': ['type: QgsProcessingParameterType'], 'parameterTypeRemoved': ['type: QgsProcessingParameterType']}
    QgsProcessingRegistry.__group__ = ['processing']
except (NameError, AttributeError):
    pass
