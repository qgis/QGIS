# The following has been generated automatically from src/core/elevation/qgsprofilesourceregistry.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProfileSourceRegistry_registerProfileSource = QgsProfileSourceRegistry.registerProfileSource
    def __QgsProfileSourceRegistry_registerProfileSource_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProfileSourceRegistry_registerProfileSource(self, arg)
    QgsProfileSourceRegistry.registerProfileSource = _functools.update_wrapper(__QgsProfileSourceRegistry_registerProfileSource_wrapper, QgsProfileSourceRegistry.registerProfileSource)

    QgsProfileSourceRegistry.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
