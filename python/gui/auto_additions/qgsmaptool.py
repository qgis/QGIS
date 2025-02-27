# The following has been generated automatically from src/gui/maptools/qgsmaptool.h
try:
    QgsMapTool.__attribute_docs__ = {'messageEmitted': 'Emitted when a ``message`` should be shown to the user in the application message bar.\n\n.. seealso:: :py:func:`messageDiscarded`\n', 'messageDiscarded': 'Emitted when the previous message from the tool should be cleared from the application message bar.\n\n.. seealso:: :py:func:`messageEmitted`\n', 'activated': 'Emitted when the map tool is activated.\n\n.. seealso:: :py:func:`deactivated`\n', 'deactivated': 'Emitted when the map tool is deactivated.\n\n.. seealso:: :py:func:`activated`\n', 'reactivated': 'Emitted when the map tool is activated, while it is already active.\n\n.. versionadded:: 3.32\n'}
    QgsMapTool.searchRadiusMM = staticmethod(QgsMapTool.searchRadiusMM)
    QgsMapTool.searchRadiusMU = staticmethod(QgsMapTool.searchRadiusMU)
    QgsMapTool.__signal_arguments__ = {'messageEmitted': ['message: str', 'level: Qgis.MessageLevel = Qgis.MessageLevel.Info']}
    QgsMapTool.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
