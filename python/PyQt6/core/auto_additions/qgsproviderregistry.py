# The following has been generated automatically from src/core/providers/qgsproviderregistry.h
QgsProviderRegistry.Standalone = QgsProviderRegistry.WidgetMode.Standalone
QgsProviderRegistry.Embedded = QgsProviderRegistry.WidgetMode.Embedded
QgsProviderRegistry.Manager = QgsProviderRegistry.WidgetMode.Manager
try:
    QgsProviderRegistry.UnusableUriDetails.__attribute_docs__ = {'uri': 'URI which could not be handled.', 'warning': 'Contains a short, user-friendly, translated message advising why the URI is not usable.', 'detailedWarning': 'Contains a longer, user-friendly, translated message advising why the URI is not usable.', 'layerTypes': 'Contains a list of map layer types which are usually valid options for opening the\ntarget URI.'}
    QgsProviderRegistry.UnusableUriDetails.__annotations__ = {'uri': str, 'warning': str, 'detailedWarning': str, 'layerTypes': 'List[Qgis.LayerType]'}
    QgsProviderRegistry.UnusableUriDetails.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.instance = staticmethod(QgsProviderRegistry.instance)
    import functools as _functools
    __wrapped_QgsProviderRegistry_registerUnusableUriHandler = QgsProviderRegistry.registerUnusableUriHandler
    def __QgsProviderRegistry_registerUnusableUriHandler_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProviderRegistry_registerUnusableUriHandler(self, arg)
    QgsProviderRegistry.registerUnusableUriHandler = _functools.update_wrapper(__QgsProviderRegistry_registerUnusableUriHandler_wrapper, QgsProviderRegistry.registerUnusableUriHandler)

    import functools as _functools
    __wrapped_QgsProviderRegistry_registerProvider = QgsProviderRegistry.registerProvider
    def __QgsProviderRegistry_registerProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProviderRegistry_registerProvider(self, arg)
    QgsProviderRegistry.registerProvider = _functools.update_wrapper(__QgsProviderRegistry_registerProvider_wrapper, QgsProviderRegistry.registerProvider)

    QgsProviderRegistry.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.UnusableUriHandlerInterface.__abstract_methods__ = ['matchesUri', 'details']
    QgsProviderRegistry.UnusableUriHandlerInterface.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.ProviderCandidateDetails.__group__ = ['providers']
except (NameError, AttributeError):
    pass
