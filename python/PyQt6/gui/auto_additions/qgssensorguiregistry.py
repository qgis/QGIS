# The following has been generated automatically from src/gui/sensor/qgssensorguiregistry.h
try:
    QgsSensorGuiRegistry.__attribute_docs__ = {'sensorAdded': 'Emitted whenever a new sensor type is added to the registry, with the\nspecified ``type``.\n'}
    QgsSensorGuiRegistry.__signal_arguments__ = {'sensorAdded': ['type: str', 'name: str']}
    import functools as _functools
    __wrapped_QgsSensorGuiRegistry_addSensorGuiMetadata = QgsSensorGuiRegistry.addSensorGuiMetadata
    def __QgsSensorGuiRegistry_addSensorGuiMetadata_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSensorGuiRegistry_addSensorGuiMetadata(self, arg)
    QgsSensorGuiRegistry.addSensorGuiMetadata = _functools.update_wrapper(__QgsSensorGuiRegistry_addSensorGuiMetadata_wrapper, QgsSensorGuiRegistry.addSensorGuiMetadata)

    QgsSensorGuiRegistry.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsSensorAbstractGuiMetadata.__virtual_methods__ = ['creationIcon', 'createSensorWidget', 'createSensor']
    QgsSensorAbstractGuiMetadata.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
