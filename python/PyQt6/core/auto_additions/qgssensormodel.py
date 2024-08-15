# The following has been generated automatically from src/core/sensor/qgssensormodel.h
# monkey patching scoped based enum
QgsSensorModel.Column.Name.__doc__ = "Name"
QgsSensorModel.Column.LastValue.__doc__ = "Last value"
QgsSensorModel.Column.__doc__ = "Model columns\n\n" + '* ``Name``: ' + QgsSensorModel.Column.Name.__doc__ + '\n' + '* ``LastValue``: ' + QgsSensorModel.Column.LastValue.__doc__
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
QgsSensorModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsSensorModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``SensorType``: ' + QgsSensorModel.CustomRole.SensorType.__doc__ + '\n' + '* ``SensorId``: ' + QgsSensorModel.CustomRole.SensorId.__doc__ + '\n' + '* ``SensorName``: ' + QgsSensorModel.CustomRole.SensorName.__doc__ + '\n' + '* ``SensorStatus``: ' + QgsSensorModel.CustomRole.SensorStatus.__doc__ + '\n' + '* ``SensorLastValue``: ' + QgsSensorModel.CustomRole.SensorLastValue.__doc__ + '\n' + '* ``SensorLastTimestamp``: ' + QgsSensorModel.CustomRole.SensorLastTimestamp.__doc__ + '\n' + '* ``Sensor``: ' + QgsSensorModel.CustomRole.Sensor.__doc__
# --
QgsSensorModel.CustomRole.baseClass = QgsSensorModel
try:
    QgsSensorModel.__group__ = ['sensor']
except NameError:
    pass
