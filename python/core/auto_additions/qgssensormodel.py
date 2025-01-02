# The following has been generated automatically from src/core/sensor/qgssensormodel.h
# monkey patching scoped based enum
QgsSensorModel.Column.Name.__doc__ = "Name"
QgsSensorModel.Column.LastValue.__doc__ = "Last value"
QgsSensorModel.Column.__doc__ = """Model columns

* ``Name``: Name
* ``LastValue``: Last value

"""
# --
QgsSensorModel.Role = QgsSensorModel.CustomRole
# monkey patching scoped based enum
QgsSensorModel.SensorType = QgsSensorModel.CustomRole.SensorType
QgsSensorModel.SensorType.is_monkey_patched = True
QgsSensorModel.SensorType.__doc__ = "Sensor type"
QgsSensorModel.SensorId = QgsSensorModel.CustomRole.SensorId
QgsSensorModel.SensorId.is_monkey_patched = True
QgsSensorModel.SensorId.__doc__ = "Sensor id"
QgsSensorModel.SensorName = QgsSensorModel.CustomRole.SensorName
QgsSensorModel.SensorName.is_monkey_patched = True
QgsSensorModel.SensorName.__doc__ = "Sensor name"
QgsSensorModel.SensorStatus = QgsSensorModel.CustomRole.SensorStatus
QgsSensorModel.SensorStatus.is_monkey_patched = True
QgsSensorModel.SensorStatus.__doc__ = "Sensor status (disconnected, connected, etc.)"
QgsSensorModel.SensorLastValue = QgsSensorModel.CustomRole.SensorLastValue
QgsSensorModel.SensorLastValue.is_monkey_patched = True
QgsSensorModel.SensorLastValue.__doc__ = "Sensor last captured value"
QgsSensorModel.SensorLastTimestamp = QgsSensorModel.CustomRole.SensorLastTimestamp
QgsSensorModel.SensorLastTimestamp.is_monkey_patched = True
QgsSensorModel.SensorLastTimestamp.__doc__ = "Sensor timestamp of last captured value"
QgsSensorModel.Sensor = QgsSensorModel.CustomRole.Sensor
QgsSensorModel.Sensor.is_monkey_patched = True
QgsSensorModel.Sensor.__doc__ = "Sensor object pointer"
QgsSensorModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsSensorModel.Role

.. versionadded:: 3.36

* ``SensorType``: Sensor type
* ``SensorId``: Sensor id
* ``SensorName``: Sensor name
* ``SensorStatus``: Sensor status (disconnected, connected, etc.)
* ``SensorLastValue``: Sensor last captured value
* ``SensorLastTimestamp``: Sensor timestamp of last captured value
* ``Sensor``: Sensor object pointer

"""
# --
QgsSensorModel.CustomRole.baseClass = QgsSensorModel
try:
    QgsSensorModel.__group__ = ['sensor']
except (NameError, AttributeError):
    pass
