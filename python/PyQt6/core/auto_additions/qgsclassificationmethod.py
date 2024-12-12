# The following has been generated automatically from src/core/classification/qgsclassificationmethod.h
QgsClassificationMethod.NoFlag = QgsClassificationMethod.MethodProperty.NoFlag
QgsClassificationMethod.ValuesNotRequired = QgsClassificationMethod.MethodProperty.ValuesNotRequired
QgsClassificationMethod.SymmetricModeAvailable = QgsClassificationMethod.MethodProperty.SymmetricModeAvailable
QgsClassificationMethod.IgnoresClassCount = QgsClassificationMethod.MethodProperty.IgnoresClassCount
QgsClassificationMethod.MethodProperties = lambda flags=0: QgsClassificationMethod.MethodProperty(flags)
QgsClassificationMethod.LowerBound = QgsClassificationMethod.ClassPosition.LowerBound
QgsClassificationMethod.Inner = QgsClassificationMethod.ClassPosition.Inner
QgsClassificationMethod.UpperBound = QgsClassificationMethod.ClassPosition.UpperBound
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsClassificationMethod.MethodProperty.__bool__ = lambda flag: bool(_force_int(flag))
QgsClassificationMethod.MethodProperty.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsClassificationMethod.MethodProperty.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsClassificationMethod.MethodProperty.__or__ = lambda flag1, flag2: QgsClassificationMethod.MethodProperty(_force_int(flag1) | _force_int(flag2))
try:
    QgsClassificationMethod.rangesToBreaks = staticmethod(QgsClassificationMethod.rangesToBreaks)
    QgsClassificationMethod.create = staticmethod(QgsClassificationMethod.create)
    QgsClassificationMethod.makeBreaksSymmetric = staticmethod(QgsClassificationMethod.makeBreaksSymmetric)
    QgsClassificationMethod.__group__ = ['classification']
except (NameError, AttributeError):
    pass
try:
    QgsClassificationRange.__group__ = ['classification']
except (NameError, AttributeError):
    pass
