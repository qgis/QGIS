# The following has been generated automatically from src/core/sensor/qgsiodevicesensor.h
try:
    QgsTcpSocketSensor.create = staticmethod(QgsTcpSocketSensor.create)
    QgsTcpSocketSensor.__overridden_methods__ = ['type', 'writePropertiesToElement', 'readPropertiesFromElement', 'handleConnect', 'handleDisconnect']
    QgsTcpSocketSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsUdpSocketSensor.create = staticmethod(QgsUdpSocketSensor.create)
    QgsUdpSocketSensor.__overridden_methods__ = ['type', 'writePropertiesToElement', 'readPropertiesFromElement', 'handleConnect', 'handleDisconnect']
    QgsUdpSocketSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsSerialPortSensor.create = staticmethod(QgsSerialPortSensor.create)
    QgsSerialPortSensor.__overridden_methods__ = ['type', 'writePropertiesToElement', 'readPropertiesFromElement', 'handleConnect', 'handleDisconnect', 'parseData']
    QgsSerialPortSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsIODeviceSensor.__virtual_methods__ = ['parseData']
    import functools as _functools
    __wrapped_QgsIODeviceSensor_initIODevice = QgsIODeviceSensor.initIODevice
    def __QgsIODeviceSensor_initIODevice_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsIODeviceSensor_initIODevice(self, arg)
    QgsIODeviceSensor.initIODevice = _functools.update_wrapper(__QgsIODeviceSensor_initIODevice_wrapper, QgsIODeviceSensor.initIODevice)

    QgsIODeviceSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
