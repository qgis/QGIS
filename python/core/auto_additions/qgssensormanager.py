# The following has been generated automatically from src/core/sensor/qgssensormanager.h
try:
    QgsSensorManager.__attribute_docs__ = {'sensorAdded': 'Emitted when a sensor has been registered.\n', 'sensorAboutToBeRemoved': 'Emitted when a sensor is about to be removed.\n', 'sensorRemoved': 'Emitted when a sensor has been removed.\n', 'sensorNameChanged': 'Emitted when a sensor name has changed.\n', 'sensorStatusChanged': 'Emitted when a sensor status has changed.\n', 'sensorDataCaptured': 'Emitted when newly captured data from a sensor has occurred.\n', 'sensorErrorOccurred': 'Emitted when a sensor error has occurred.\n'}
    QgsSensorManager.__signal_arguments__ = {'sensorAdded': ['id: str'], 'sensorAboutToBeRemoved': ['id: str'], 'sensorRemoved': ['id: str'], 'sensorNameChanged': ['id: str'], 'sensorStatusChanged': ['id: str'], 'sensorDataCaptured': ['id: str'], 'sensorErrorOccurred': ['id: str']}
    import functools as _functools
    __wrapped_QgsSensorManager_addSensor = QgsSensorManager.addSensor
    def __QgsSensorManager_addSensor_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsSensorManager_addSensor(self, arg)
    QgsSensorManager.addSensor = _functools.update_wrapper(__QgsSensorManager_addSensor_wrapper, QgsSensorManager.addSensor)

    QgsSensorManager.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
