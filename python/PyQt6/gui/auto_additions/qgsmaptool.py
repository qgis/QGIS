# The following has been generated automatically from src/gui/maptools/qgsmaptool.h
QgsMapTool.Transient = QgsMapTool.Flag.Transient
QgsMapTool.EditTool = QgsMapTool.Flag.EditTool
QgsMapTool.AllowZoomRect = QgsMapTool.Flag.AllowZoomRect
QgsMapTool.ShowContextMenu = QgsMapTool.Flag.ShowContextMenu
QgsMapTool.Flags = lambda flags=0: QgsMapTool.Flag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsMapTool.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapTool.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapTool.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapTool.Flag.__or__ = lambda flag1, flag2: QgsMapTool.Flag(_force_int(flag1) | _force_int(flag2))
try:
    QgsMapTool.__attribute_docs__ = {'messageEmitted': 'emit a message\n', 'messageDiscarded': 'emit signal to clear previous message\n', 'activated': 'signal emitted once the map tool is activated\n', 'deactivated': 'signal emitted once the map tool is deactivated\n', 'reactivated': '\n.. versionadded:: 3.32\n'}
    QgsMapTool.searchRadiusMM = staticmethod(QgsMapTool.searchRadiusMM)
    QgsMapTool.searchRadiusMU = staticmethod(QgsMapTool.searchRadiusMU)
    QgsMapTool.__signal_arguments__ = {'messageEmitted': ['message: str', ': Qgis.MessageLevel = Qgis.MessageLevel.Info']}
    QgsMapTool.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
