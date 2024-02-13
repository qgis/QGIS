# The following has been generated automatically from src/core/qgsfeaturesink.h
QgsFeatureSink.RegeneratePrimaryKey = QgsFeatureSink.SinkFlag.RegeneratePrimaryKey
QgsFeatureSink.SinkFlags = lambda flags=0: QgsFeatureSink.SinkFlag(flags)
QgsFeatureSink.FastInsert = QgsFeatureSink.Flag.FastInsert
QgsFeatureSink.RollBackOnErrors = QgsFeatureSink.Flag.RollBackOnErrors
QgsFeatureSink.Flags = lambda flags=0: QgsFeatureSink.Flag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsFeatureSink.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsFeatureSink.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFeatureSink.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFeatureSink.Flag.__or__ = lambda flag1, flag2: QgsFeatureSink.Flag(_force_int(flag1) | _force_int(flag2))
