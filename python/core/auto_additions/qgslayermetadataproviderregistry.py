# The following has been generated automatically from src/core/metadata/qgslayermetadataproviderregistry.h
try:
    import functools as _functools
    __wrapped_QgsLayerMetadataProviderRegistry_registerLayerMetadataProvider = QgsLayerMetadataProviderRegistry.registerLayerMetadataProvider
    def __QgsLayerMetadataProviderRegistry_registerLayerMetadataProvider_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayerMetadataProviderRegistry_registerLayerMetadataProvider(self, arg)
    QgsLayerMetadataProviderRegistry.registerLayerMetadataProvider = _functools.update_wrapper(__QgsLayerMetadataProviderRegistry_registerLayerMetadataProvider_wrapper, QgsLayerMetadataProviderRegistry.registerLayerMetadataProvider)

    QgsLayerMetadataProviderRegistry.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
