# The following has been generated automatically from src/gui/editorwidgets/core/qgssearchwidgetwrapper.h
QgsSearchWidgetWrapper.EqualTo = QgsSearchWidgetWrapper.FilterFlag.EqualTo
QgsSearchWidgetWrapper.NotEqualTo = QgsSearchWidgetWrapper.FilterFlag.NotEqualTo
QgsSearchWidgetWrapper.GreaterThan = QgsSearchWidgetWrapper.FilterFlag.GreaterThan
QgsSearchWidgetWrapper.LessThan = QgsSearchWidgetWrapper.FilterFlag.LessThan
QgsSearchWidgetWrapper.GreaterThanOrEqualTo = QgsSearchWidgetWrapper.FilterFlag.GreaterThanOrEqualTo
QgsSearchWidgetWrapper.LessThanOrEqualTo = QgsSearchWidgetWrapper.FilterFlag.LessThanOrEqualTo
QgsSearchWidgetWrapper.Between = QgsSearchWidgetWrapper.FilterFlag.Between
QgsSearchWidgetWrapper.CaseInsensitive = QgsSearchWidgetWrapper.FilterFlag.CaseInsensitive
QgsSearchWidgetWrapper.Contains = QgsSearchWidgetWrapper.FilterFlag.Contains
QgsSearchWidgetWrapper.DoesNotContain = QgsSearchWidgetWrapper.FilterFlag.DoesNotContain
QgsSearchWidgetWrapper.IsNull = QgsSearchWidgetWrapper.FilterFlag.IsNull
QgsSearchWidgetWrapper.IsNotBetween = QgsSearchWidgetWrapper.FilterFlag.IsNotBetween
QgsSearchWidgetWrapper.IsNotNull = QgsSearchWidgetWrapper.FilterFlag.IsNotNull
QgsSearchWidgetWrapper.StartsWith = QgsSearchWidgetWrapper.FilterFlag.StartsWith
QgsSearchWidgetWrapper.EndsWith = QgsSearchWidgetWrapper.FilterFlag.EndsWith
QgsSearchWidgetWrapper.FilterFlags = lambda flags=0: QgsSearchWidgetWrapper.FilterFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsSearchWidgetWrapper.FilterFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsSearchWidgetWrapper.FilterFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsSearchWidgetWrapper.FilterFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsSearchWidgetWrapper.FilterFlag.__or__ = lambda flag1, flag2: QgsSearchWidgetWrapper.FilterFlag(_force_int(flag1) | _force_int(flag2))
try:
    QgsSearchWidgetWrapper.__attribute_docs__ = {'expressionChanged': 'Emitted whenever the expression changes\n\n:param exp: The new search expression\n', 'valueChanged': 'Emitted when a user changes the value of the search widget.\n', 'valueCleared': 'Emitted when a user changes the value of the search widget back\nto an empty, default state.\n'}
    QgsSearchWidgetWrapper.exclusiveFilterFlags = staticmethod(QgsSearchWidgetWrapper.exclusiveFilterFlags)
    QgsSearchWidgetWrapper.nonExclusiveFilterFlags = staticmethod(QgsSearchWidgetWrapper.nonExclusiveFilterFlags)
    QgsSearchWidgetWrapper.toString = staticmethod(QgsSearchWidgetWrapper.toString)
    QgsSearchWidgetWrapper.__signal_arguments__ = {'expressionChanged': ['exp: str']}
    QgsSearchWidgetWrapper.__group__ = ['editorwidgets', 'core']
except (NameError, AttributeError):
    pass
