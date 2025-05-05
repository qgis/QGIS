# The following has been generated automatically from src/core/sensor/qgssensorregistry.h
try:
    QgsSensorRegistry.__attribute_docs__ = {'sensorAdded': 'Emitted whenever a new sensor type is added to the registry, with the\nspecified ``type`` and visible ``name``.\n'}
    QgsSensorRegistry.__signal_arguments__ = {'sensorAdded': ['type: str', 'name: str']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsSensorRegistry_addSensorType = QgsSensorRegistry.addSensorType
    def __QgsSensorRegistry_addSensorType_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSensorRegistry_addSensorType(self, arg)
    QgsSensorRegistry.addSensorType = _functools.update_wrapper(__QgsSensorRegistry_addSensorType_wrapper, QgsSensorRegistry.addSensorType)

    QgsSensorRegistry.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsSensorAbstractMetadata.__abstract_methods__ = ['createSensor']
    QgsSensorAbstractMetadata.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
