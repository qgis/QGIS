# The following has been generated automatically from src/core/qgsvectorsimplifymethod.h
QgsVectorSimplifyMethod.NoSimplification = QgsVectorSimplifyMethod.SimplifyHint.NoSimplification
QgsVectorSimplifyMethod.GeometrySimplification = QgsVectorSimplifyMethod.SimplifyHint.GeometrySimplification
QgsVectorSimplifyMethod.AntialiasingSimplification = QgsVectorSimplifyMethod.SimplifyHint.AntialiasingSimplification
QgsVectorSimplifyMethod.FullSimplification = QgsVectorSimplifyMethod.SimplifyHint.FullSimplification
QgsVectorSimplifyMethod.SimplifyHint.baseClass = QgsVectorSimplifyMethod
QgsVectorSimplifyMethod.SimplifyHints = lambda flags=0: QgsVectorSimplifyMethod.SimplifyHint(flags)
QgsVectorSimplifyMethod.SimplifyHints.baseClass = QgsVectorSimplifyMethod
SimplifyHints = QgsVectorSimplifyMethod  # dirty hack since SIP seems to introduce the flags in module
QgsVectorSimplifyMethod.Distance = QgsVectorSimplifyMethod.SimplifyAlgorithm.Distance
QgsVectorSimplifyMethod.SnapToGrid = QgsVectorSimplifyMethod.SimplifyAlgorithm.SnapToGrid
QgsVectorSimplifyMethod.Visvalingam = QgsVectorSimplifyMethod.SimplifyAlgorithm.Visvalingam
QgsVectorSimplifyMethod.SnappedToGridGlobal = QgsVectorSimplifyMethod.SimplifyAlgorithm.SnappedToGridGlobal
QgsVectorSimplifyMethod.SimplifyAlgorithm.baseClass = QgsVectorSimplifyMethod
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsVectorSimplifyMethod.SimplifyHint.__bool__ = lambda flag: bool(_force_int(flag))
QgsVectorSimplifyMethod.SimplifyHint.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsVectorSimplifyMethod.SimplifyHint.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsVectorSimplifyMethod.SimplifyHint.__or__ = lambda flag1, flag2: QgsVectorSimplifyMethod.SimplifyHint(_force_int(flag1) | _force_int(flag2))
