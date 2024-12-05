# The following has been generated automatically from src/core/sensor/qgsabstractsensor.h
try:
    QgsAbstractSensor.SensorData.__attribute_docs__ = {'lastValue': 'Last captured sensor value stored as a QVariant.\n\n.. note::\n\n   The member can store multiple values if the sensor passes on a QVariantMap.', 'lastTimestamp': 'Timestamp of last captured sensor value'}
    QgsAbstractSensor.SensorData.__doc__ = """Contains details of a sensor data capture"""
    QgsAbstractSensor.SensorData.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractSensor.__attribute_docs__ = {'nameChanged': 'Emitted when the sensor name has changed.\n', 'statusChanged': 'Emitted when the sensor status has changed.\n', 'dataChanged': 'Emitted when the captured sensor data has changed.\n', 'errorOccurred': 'Emitted when an error has occurred. The ``errorString`` describes the error.\n'}
    QgsAbstractSensor.__signal_arguments__ = {'errorOccurred': ['errorString: str']}
    QgsAbstractSensor.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
