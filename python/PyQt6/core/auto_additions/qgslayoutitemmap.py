# The following has been generated automatically from src/core/layout/qgslayoutitemmap.h
QgsLayoutItemMap.Fixed = QgsLayoutItemMap.AtlasScalingMode.Fixed
QgsLayoutItemMap.Predefined = QgsLayoutItemMap.AtlasScalingMode.Predefined
QgsLayoutItemMap.Auto = QgsLayoutItemMap.AtlasScalingMode.Auto
QgsLayoutItemMap.ShowPartialLabels = QgsLayoutItemMap.MapItemFlag.ShowPartialLabels
QgsLayoutItemMap.ShowUnplacedLabels = QgsLayoutItemMap.MapItemFlag.ShowUnplacedLabels
QgsLayoutItemMap.MapItemFlags = lambda flags=0: QgsLayoutItemMap.MapItemFlag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsLayoutItemMap.MapItemFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsLayoutItemMap.MapItemFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsLayoutItemMap.MapItemFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsLayoutItemMap.MapItemFlag.__or__ = lambda flag1, flag2: QgsLayoutItemMap.MapItemFlag(_force_int(flag1) | _force_int(flag2))
