# The following has been generated automatically from src/core/classification/qgsclassificationmethod.h
QgsClassificationMethod.NoFlag = QgsClassificationMethod.MethodProperty.NoFlag
QgsClassificationMethod.ValuesNotRequired = QgsClassificationMethod.MethodProperty.ValuesNotRequired
QgsClassificationMethod.SymmetricModeAvailable = QgsClassificationMethod.MethodProperty.SymmetricModeAvailable
QgsClassificationMethod.IgnoresClassCount = QgsClassificationMethod.MethodProperty.IgnoresClassCount
QgsClassificationMethod.MethodProperties = lambda flags=0: QgsClassificationMethod.MethodProperty(flags)
QgsClassificationMethod.LowerBound = QgsClassificationMethod.ClassPosition.LowerBound
QgsClassificationMethod.Inner = QgsClassificationMethod.ClassPosition.Inner
QgsClassificationMethod.UpperBound = QgsClassificationMethod.ClassPosition.UpperBound
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsClassificationMethod.MethodProperty.__bool__ = lambda flag: bool(_force_int(flag))
QgsClassificationMethod.MethodProperty.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsClassificationMethod.MethodProperty.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsClassificationMethod.MethodProperty.__or__ = lambda flag1, flag2: QgsClassificationMethod.MethodProperty(_force_int(flag1) | _force_int(flag2))
