# The following has been generated automatically from src/gui/processing/qgsprocessingguiregistry.h
try:
    import functools as _functools
    __wrapped_QgsProcessingGuiRegistry_addAlgorithmConfigurationWidgetFactory = QgsProcessingGuiRegistry.addAlgorithmConfigurationWidgetFactory
    def __QgsProcessingGuiRegistry_addAlgorithmConfigurationWidgetFactory_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingGuiRegistry_addAlgorithmConfigurationWidgetFactory(self, arg)
    QgsProcessingGuiRegistry.addAlgorithmConfigurationWidgetFactory = _functools.update_wrapper(__QgsProcessingGuiRegistry_addAlgorithmConfigurationWidgetFactory_wrapper, QgsProcessingGuiRegistry.addAlgorithmConfigurationWidgetFactory)

    import functools as _functools
    __wrapped_QgsProcessingGuiRegistry_addParameterWidgetFactory = QgsProcessingGuiRegistry.addParameterWidgetFactory
    def __QgsProcessingGuiRegistry_addParameterWidgetFactory_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingGuiRegistry_addParameterWidgetFactory(self, arg)
    QgsProcessingGuiRegistry.addParameterWidgetFactory = _functools.update_wrapper(__QgsProcessingGuiRegistry_addParameterWidgetFactory_wrapper, QgsProcessingGuiRegistry.addParameterWidgetFactory)

    QgsProcessingGuiRegistry.__group__ = ['processing']
except (NameError, AttributeError):
    pass
