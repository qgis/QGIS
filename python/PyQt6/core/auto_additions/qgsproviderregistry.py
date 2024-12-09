# The following has been generated automatically from src/core/providers/qgsproviderregistry.h
QgsProviderRegistry.Standalone = QgsProviderRegistry.WidgetMode.Standalone
QgsProviderRegistry.Embedded = QgsProviderRegistry.WidgetMode.Embedded
QgsProviderRegistry.Manager = QgsProviderRegistry.WidgetMode.Manager
try:
    QgsProviderRegistry.UnusableUriDetails.__attribute_docs__ = {'uri': 'URI which could not be handled.', 'warning': 'Contains a short, user-friendly, translated message advising why the URI is not usable.', 'detailedWarning': 'Contains a longer, user-friendly, translated message advising why the URI is not usable.', 'layerTypes': 'Contains a list of map layer types which are usually valid options for opening the\ntarget URI.'}
    QgsProviderRegistry.UnusableUriDetails.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.instance = staticmethod(QgsProviderRegistry.instance)
    QgsProviderRegistry.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.ProviderCandidateDetails.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderRegistry.UnusableUriHandlerInterface.__group__ = ['providers']
except (NameError, AttributeError):
    pass
