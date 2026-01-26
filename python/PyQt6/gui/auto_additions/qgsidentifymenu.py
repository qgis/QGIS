# The following has been generated automatically from src/gui/qgsidentifymenu.h
QgsIdentifyMenu.LayerLevel = QgsIdentifyMenu.MenuLevel.LayerLevel
QgsIdentifyMenu.FeatureLevel = QgsIdentifyMenu.MenuLevel.FeatureLevel
try:
    QgsIdentifyMenu.__attribute_docs__ = {'messageEmitted': 'Emitted when a ``message`` should be shown to the user in the\napplication message bar.\n\n.. seealso:: :py:func:`messageDiscarded`\n\n.. versionadded:: 4.0\n', 'messageDiscarded': 'Emitted when the previous message from the tool should be cleared from\nthe application message bar.\n\n.. seealso:: :py:func:`messageEmitted`\n\n.. versionadded:: 4.0\n'}
    QgsIdentifyMenu.findFeaturesOnCanvas = staticmethod(QgsIdentifyMenu.findFeaturesOnCanvas)
    QgsIdentifyMenu.styleHighlight = staticmethod(QgsIdentifyMenu.styleHighlight)
    QgsIdentifyMenu.__overridden_methods__ = ['closeEvent']
    QgsIdentifyMenu.__signal_arguments__ = {'messageEmitted': ['message: str', 'level: Qgis.MessageLevel = Qgis.MessageLevel.Info']}
except (NameError, AttributeError):
    pass
