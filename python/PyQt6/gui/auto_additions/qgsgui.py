# The following has been generated automatically from src/gui/qgsgui.h
QgsGui.UseCrsOfFirstLayerAdded = QgsGui.ProjectCrsBehavior.UseCrsOfFirstLayerAdded
QgsGui.UsePresetCrs = QgsGui.ProjectCrsBehavior.UsePresetCrs
QgsGui.ProjectCrsBehavior.baseClass = QgsGui
QgsGui.HigMenuTextIsTitleCase = QgsGui.HigFlag.HigMenuTextIsTitleCase
QgsGui.HigDialogTitleIsTitleCase = QgsGui.HigFlag.HigDialogTitleIsTitleCase
QgsGui.HigFlags = lambda flags=0: QgsGui.HigFlag(flags)
_force_int = lambda v: v if isinstance(v, int) else int(v.value)


QgsGui.HigFlag.__bool__ = lambda flag: _force_int(flag)
QgsGui.HigFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsGui.HigFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsGui.HigFlag.__or__ = lambda flag1, flag2: QgsGui.HigFlag(_force_int(flag1) | _force_int(flag2))
