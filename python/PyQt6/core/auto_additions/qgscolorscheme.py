# The following has been generated automatically from src/core/qgscolorscheme.h
QgsColorScheme.ShowInColorDialog = QgsColorScheme.SchemeFlag.ShowInColorDialog
QgsColorScheme.ShowInColorButtonMenu = QgsColorScheme.SchemeFlag.ShowInColorButtonMenu
QgsColorScheme.ShowInAllContexts = QgsColorScheme.SchemeFlag.ShowInAllContexts
QgsColorScheme.SchemeFlags = lambda flags=0: QgsColorScheme.SchemeFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsColorScheme.SchemeFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsColorScheme.SchemeFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsColorScheme.SchemeFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsColorScheme.SchemeFlag.__or__ = lambda flag1, flag2: QgsColorScheme.SchemeFlag(_force_int(flag1) | _force_int(flag2))
try:
    QgsRecentColorScheme.addRecentColor = staticmethod(QgsRecentColorScheme.addRecentColor)
    QgsRecentColorScheme.lastUsedColor = staticmethod(QgsRecentColorScheme.lastUsedColor)
    QgsRecentColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsColorScheme.__virtual_methods__ = ['flags', 'fetchColors', 'isEditable', 'setColors']
    QgsColorScheme.__abstract_methods__ = ['schemeName', 'fetchColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsGplColorScheme.__abstract_methods__ = ['gplFilePath']
    QgsGplColorScheme.__overridden_methods__ = ['fetchColors', 'setColors']
except (NameError, AttributeError):
    pass
try:
    QgsUserColorScheme.__overridden_methods__ = ['schemeName', 'clone', 'isEditable', 'flags', 'gplFilePath']
except (NameError, AttributeError):
    pass
try:
    QgsCustomColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'isEditable', 'setColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsProjectColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'isEditable', 'setColors', 'clone']
except (NameError, AttributeError):
    pass
