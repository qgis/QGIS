# The following has been generated automatically from src/gui/qgsextentwidget.h
try:
    QgsExtentWidget.__attribute_docs__ = {'extentChanged': "Emitted when the widget's extent is changed.\n", 'validationChanged': "Emitted when the widget's validation state changes.\n", 'toggleDialogVisibility': 'Emitted when the parent dialog visibility must be changed (e.g. to\npermit access to the map canvas)\n', 'extentLayerChanged': 'Emitted when the extent layer is changed.\n\n.. versionadded:: 3.44\n', 'snapToGridChanged': 'Emitted when the snap-to-grid state is changed.\n\n:param enabled: whether snap-to-grid is enabled\n\n.. versionadded:: 3.46\n'}
    QgsExtentWidget.__overridden_methods__ = ['dragEnterEvent', 'dragLeaveEvent', 'dropEvent', 'showEvent']
    QgsExtentWidget.__signal_arguments__ = {'extentChanged': ['r: QgsRectangle'], 'validationChanged': ['valid: bool'], 'toggleDialogVisibility': ['visible: bool'], 'extentLayerChanged': ['layer: QgsMapLayer'], 'snapToGridChanged': ['enabled: bool']}
except (NameError, AttributeError):
    pass
