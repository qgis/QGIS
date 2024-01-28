# The following has been generated automatically from src/gui/processing/models/qgsmodelcomponentgraphicitem.h
QgsModelComponentGraphicItem.Normal = QgsModelComponentGraphicItem.State.Normal
QgsModelComponentGraphicItem.Selected = QgsModelComponentGraphicItem.State.Selected
QgsModelComponentGraphicItem.Hover = QgsModelComponentGraphicItem.State.Hover
QgsModelComponentGraphicItem.Unused = QgsModelComponentGraphicItem.Flag.Unused
QgsModelComponentGraphicItem.Flags = lambda flags=0: QgsModelComponentGraphicItem.Flag(flags)
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsModelComponentGraphicItem.Flag.__bool__ = lambda flag: _force_int(flag)
QgsModelComponentGraphicItem.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsModelComponentGraphicItem.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsModelComponentGraphicItem.Flag.__or__ = lambda flag1, flag2: QgsModelComponentGraphicItem.Flag(_force_int(flag1) | _force_int(flag2))
