# The following has been generated automatically from src/core/auth/qgsauthconfigurationstorageregistry.h
try:
    QgsAuthConfigurationStorageRegistry.__attribute_docs__ = {'storageAdded': 'Emitted after a storage was added\n\n:param id: The id of the added storage\n', 'storageChanged': 'Emitted after a storage was changed\n\n:param id: The id of the changed storage\n', 'storageRemoved': 'Emitted after a storage was removed\n\n:param id: The id of the removed storage\n'}
    QgsAuthConfigurationStorageRegistry.__signal_arguments__ = {'storageAdded': ['id: str'], 'storageChanged': ['id: str'], 'storageRemoved': ['id: str']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAuthConfigurationStorageRegistry_addStorage = QgsAuthConfigurationStorageRegistry.addStorage
    def __QgsAuthConfigurationStorageRegistry_addStorage_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAuthConfigurationStorageRegistry_addStorage(self, arg)
    QgsAuthConfigurationStorageRegistry.addStorage = _functools.update_wrapper(__QgsAuthConfigurationStorageRegistry_addStorage_wrapper, QgsAuthConfigurationStorageRegistry.addStorage)

    QgsAuthConfigurationStorageRegistry.__group__ = ['auth']
except (NameError, AttributeError):
    pass
