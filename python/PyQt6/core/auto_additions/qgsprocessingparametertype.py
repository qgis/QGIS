# The following has been generated automatically from src/core/processing/qgsprocessingparametertype.h
QgsProcessingParameterType.ExposeToModeler = QgsProcessingParameterType.ParameterFlag.ExposeToModeler
QgsProcessingParameterType.ParameterFlags = lambda flags=0: QgsProcessingParameterType.ParameterFlag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsProcessingParameterType.ParameterFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsProcessingParameterType.ParameterFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingParameterType.ParameterFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingParameterType.ParameterFlag.__or__ = lambda flag1, flag2: QgsProcessingParameterType.ParameterFlag(_force_int(flag1) | _force_int(flag2))
