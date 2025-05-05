# The following has been generated automatically from src/core/browser/qgsdataitemproviderregistry.h
try:
    QgsDataItemProviderRegistry.__attribute_docs__ = {'providerAdded': 'Emitted when a new data item provider has been added.\n\n.. versionadded:: 3.14\n', 'providerWillBeRemoved': 'Emitted when a data item provider is about to be removed\n\n.. versionadded:: 3.14\n'}
    QgsDataItemProviderRegistry.__signal_arguments__ = {'providerAdded': ['provider: QgsDataItemProvider'], 'providerWillBeRemoved': ['provider: QgsDataItemProvider']}
    import functools as _functools
    __wrapped_QgsDataItemProviderRegistry_addProvider = QgsDataItemProviderRegistry.addProvider
    def __QgsDataItemProviderRegistry_addProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsDataItemProviderRegistry_addProvider(self, arg)
    QgsDataItemProviderRegistry.addProvider = _functools.update_wrapper(__QgsDataItemProviderRegistry_addProvider_wrapper, QgsDataItemProviderRegistry.addProvider)

    QgsDataItemProviderRegistry.__group__ = ['browser']
except (NameError, AttributeError):
    pass
