# The following has been generated automatically from src/gui/layertree/qgslayertreeembeddedwidgetregistry.h
try:
    QgsLayerTreeEmbeddedWidgetProvider.__abstract_methods__ = ['id', 'name', 'createWidget', 'supportsLayer']
    QgsLayerTreeEmbeddedWidgetProvider.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayerTreeEmbeddedWidgetRegistry_addProvider = QgsLayerTreeEmbeddedWidgetRegistry.addProvider
    def __QgsLayerTreeEmbeddedWidgetRegistry_addProvider_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayerTreeEmbeddedWidgetRegistry_addProvider(self, arg)
    QgsLayerTreeEmbeddedWidgetRegistry.addProvider = _functools.update_wrapper(__QgsLayerTreeEmbeddedWidgetRegistry_addProvider_wrapper, QgsLayerTreeEmbeddedWidgetRegistry.addProvider)

    QgsLayerTreeEmbeddedWidgetRegistry.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
