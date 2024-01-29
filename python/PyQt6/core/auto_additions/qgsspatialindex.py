# The following has been generated automatically from src/core/qgsspatialindex.h
QgsSpatialIndex.FlagStoreFeatureGeometries = QgsSpatialIndex.Flag.FlagStoreFeatureGeometries
QgsSpatialIndex.Flags = lambda flags=0: QgsSpatialIndex.Flag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsSpatialIndex.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsSpatialIndex.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsSpatialIndex.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsSpatialIndex.Flag.__or__ = lambda flag1, flag2: QgsSpatialIndex.Flag(_force_int(flag1) | _force_int(flag2))
