# The following has been generated automatically from src/core/processing/qgsprocessingregistry.h
try:
    QgsProcessingAlgorithmInformation.__attribute_docs__ = {'displayName': 'Algorithm display name', 'icon': 'Algorithm icon'}
    QgsProcessingAlgorithmInformation.__annotations__ = {'displayName': str, 'icon': 'QIcon'}
    QgsProcessingAlgorithmInformation.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingRegistry.__attribute_docs__ = {'providerAdded': 'Emitted when a provider has been added to the registry.\n', 'providerRemoved': 'Emitted when a provider is removed from the registry\n', 'parameterTypeAdded': 'Emitted when a new parameter type has been added to the registry.\n\n.. versionadded:: 3.2\n', 'parameterTypeRemoved': 'Emitted when a parameter type has been removed from the registry and is\nabout to be deleted.\n\n.. versionadded:: 3.2\n'}
    QgsProcessingRegistry.__signal_arguments__ = {'providerAdded': ['id: str'], 'providerRemoved': ['id: str'], 'parameterTypeAdded': ['type: QgsProcessingParameterType'], 'parameterTypeRemoved': ['type: QgsProcessingParameterType']}
    import functools as _functools
    __wrapped_QgsProcessingRegistry_addProvider = QgsProcessingRegistry.addProvider
    def __QgsProcessingRegistry_addProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingRegistry_addProvider(self, arg)
    QgsProcessingRegistry.addProvider = _functools.update_wrapper(__QgsProcessingRegistry_addProvider_wrapper, QgsProcessingRegistry.addProvider)

    import functools as _functools
    __wrapped_QgsProcessingRegistry_addParameterType = QgsProcessingRegistry.addParameterType
    def __QgsProcessingRegistry_addParameterType_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingRegistry_addParameterType(self, arg)
    QgsProcessingRegistry.addParameterType = _functools.update_wrapper(__QgsProcessingRegistry_addParameterType_wrapper, QgsProcessingRegistry.addParameterType)

    QgsProcessingRegistry.__group__ = ['processing']
except (NameError, AttributeError):
    pass
