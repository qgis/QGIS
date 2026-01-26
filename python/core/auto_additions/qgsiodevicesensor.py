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
    QgsIODeviceSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
