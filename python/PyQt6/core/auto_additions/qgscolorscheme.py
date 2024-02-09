# The following has been generated automatically from src/core/qgscolorscheme.h
QgsColorScheme.ShowInColorDialog = QgsColorScheme.SchemeFlag.ShowInColorDialog
QgsColorScheme.ShowInColorButtonMenu = QgsColorScheme.SchemeFlag.ShowInColorButtonMenu
QgsColorScheme.ShowInAllContexts = QgsColorScheme.SchemeFlag.ShowInAllContexts
QgsColorScheme.SchemeFlags = lambda flags=0: QgsColorScheme.SchemeFlag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsColorScheme.SchemeFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsColorScheme.SchemeFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsColorScheme.SchemeFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsColorScheme.SchemeFlag.__or__ = lambda flag1, flag2: QgsColorScheme.SchemeFlag(_force_int(flag1) | _force_int(flag2))
