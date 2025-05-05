# The following has been generated automatically from src/core/project/qgsprojectstorageregistry.h
try:
    import functools as _functools
    __wrapped_QgsProjectStorageRegistry_registerProjectStorage = QgsProjectStorageRegistry.registerProjectStorage
    def __QgsProjectStorageRegistry_registerProjectStorage_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProjectStorageRegistry_registerProjectStorage(self, arg)
    QgsProjectStorageRegistry.registerProjectStorage = _functools.update_wrapper(__QgsProjectStorageRegistry_registerProjectStorage_wrapper, QgsProjectStorageRegistry.registerProjectStorage)

    QgsProjectStorageRegistry.__group__ = ['project']
except (NameError, AttributeError):
    pass
