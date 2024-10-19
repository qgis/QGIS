# The following has been generated automatically from src/gui/qgspanelwidget.h
try:
    QgsPanelWidget.__attribute_docs__ = {'panelAccepted': 'Emitted when the panel is accepted by the user.\n\n:param panel: The panel widget that was accepted.\n\n.. note::\n\n   This argument is normally raised with emit panelAccepted(this)\n   so that callers can retrieve the widget easier in calling code.\n\n.. note::\n\n   this is emitted only when this panel is accepted, and is not emitted for\n   child panels. For example, if this panel opens a second stacked panel, then this panel\n   will not emit panelAccepted when the second panel is accepted.\n', 'showPanel': 'Emit when you require a panel to be show in the interface.\n\n:param panel: The panel widget to show.\n\n.. note::\n\n   If you are connected to this signal you should also connect\n   given panels showPanel signal as they can be nested.\n', 'widgetChanged': 'Emitted when the widget state changes.\nConnect to this to pull any changes off the widget when needed.\nAs panels are non blocking "dialogs" you should listen to this signal\nto give the user feedback when something changes.\n'}
    QgsPanelWidget.findParentPanel = staticmethod(QgsPanelWidget.findParentPanel)
    QgsPanelWidget.__signal_arguments__ = {'panelAccepted': ['panel: QgsPanelWidget'], 'showPanel': ['panel: QgsPanelWidget']}
except NameError:
    pass
