# The following has been generated automatically from src/core/qgspluginlayerregistry.h
try:
    QgsPluginLayerType.__virtual_methods__ = ['createLayer', 'showLayerProperties']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPluginLayerRegistry_addPluginLayerType = QgsPluginLayerRegistry.addPluginLayerType
    def __QgsPluginLayerRegistry_addPluginLayerType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPluginLayerRegistry_addPluginLayerType(self, arg)
    QgsPluginLayerRegistry.addPluginLayerType = _functools.update_wrapper(__QgsPluginLayerRegistry_addPluginLayerType_wrapper, QgsPluginLayerRegistry.addPluginLayerType)

except (NameError, AttributeError):
    pass
