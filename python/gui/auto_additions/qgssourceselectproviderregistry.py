# The following has been generated automatically from src/gui/qgssourceselectproviderregistry.h
try:
    QgsSourceSelectProviderRegistry.__attribute_docs__ = {'providerAdded': 'Emitted whenever a provider is added to the registry.\n\n.. versionadded:: 3.30\n', 'providerRemoved': 'Emitted whenever a provider is removed from the registry.\n\n.. versionadded:: 3.30\n'}
    QgsSourceSelectProviderRegistry.__signal_arguments__ = {'providerAdded': ['name: str'], 'providerRemoved': ['name: str']}
    import functools as _functools
    __wrapped_QgsSourceSelectProviderRegistry_addProvider = QgsSourceSelectProviderRegistry.addProvider
    def __QgsSourceSelectProviderRegistry_addProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSourceSelectProviderRegistry_addProvider(self, arg)
    QgsSourceSelectProviderRegistry.addProvider = _functools.update_wrapper(__QgsSourceSelectProviderRegistry_addProvider_wrapper, QgsSourceSelectProviderRegistry.addProvider)

    import functools as _functools
    __wrapped_QgsSourceSelectProviderRegistry_removeProvider = QgsSourceSelectProviderRegistry.removeProvider
    def __QgsSourceSelectProviderRegistry_removeProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSourceSelectProviderRegistry_removeProvider(self, arg)
    QgsSourceSelectProviderRegistry.removeProvider = _functools.update_wrapper(__QgsSourceSelectProviderRegistry_removeProvider_wrapper, QgsSourceSelectProviderRegistry.removeProvider)

except (NameError, AttributeError):
    pass
