# The following has been generated automatically from src/gui/actions/qgsactionmenu.h
try:
    QgsActionMenu.__attribute_docs__ = {'reinit': 'Emitted after actions have been reloaded.\n', 'messageEmitted': 'Emitted when a ``message`` should be shown to the user in the\napplication message bar.\n\n.. seealso:: :py:func:`messageDiscarded`\n\n.. versionadded:: 4.0\n', 'messageDiscarded': 'Emitted when the previous message from the tool should be cleared from\nthe application message bar.\n\n.. seealso:: :py:func:`messageEmitted`\n\n.. versionadded:: 4.0\n'}
    QgsActionMenu.__signal_arguments__ = {'messageEmitted': ['message: str', 'level: Qgis.MessageLevel = Qgis.MessageLevel.Info']}
    QgsActionMenu.__group__ = ['actions']
except (NameError, AttributeError):
    pass
try:
    QgsActionMenu.ActionData.__group__ = ['actions']
except (NameError, AttributeError):
    pass
