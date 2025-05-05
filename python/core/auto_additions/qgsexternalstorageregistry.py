# The following has been generated automatically from src/core/externalstorage/qgsexternalstorageregistry.h
try:
    import functools as _functools
    __wrapped_QgsExternalStorageRegistry_registerExternalStorage = QgsExternalStorageRegistry.registerExternalStorage
    def __QgsExternalStorageRegistry_registerExternalStorage_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsExternalStorageRegistry_registerExternalStorage(self, arg)
    QgsExternalStorageRegistry.registerExternalStorage = _functools.update_wrapper(__QgsExternalStorageRegistry_registerExternalStorage_wrapper, QgsExternalStorageRegistry.registerExternalStorage)

    QgsExternalStorageRegistry.__group__ = ['externalstorage']
except (NameError, AttributeError):
    pass
