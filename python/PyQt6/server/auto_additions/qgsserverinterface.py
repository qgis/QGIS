# The following has been generated automatically from src/server/qgsserverinterface.h
try:
    QgsServerInterface.__abstract_methods__ = ['capabilitiesCache', 'requestHandler', 'registerFilter', 'setFilters', 'filters', 'registerAccessControl', 'accessControls', 'registerServerCache', 'cacheManager', 'getEnv', 'configFilePath', 'setConfigFilePath', 'removeConfigCacheEntry', 'serviceRegistry', 'reloadSettings']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsServerInterface_setFilters = QgsServerInterface.setFilters
    def __QgsServerInterface_setFilters_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsServerInterface_setFilters(self, arg)
    QgsServerInterface.setFilters = _functools.update_wrapper(__QgsServerInterface_setFilters_wrapper, QgsServerInterface.setFilters)

except (NameError, AttributeError):
    pass
