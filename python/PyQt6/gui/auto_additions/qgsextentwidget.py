# The following has been generated automatically from src/gui/qgsextentwidget.h
QgsExtentWidget.OriginalExtent = QgsExtentWidget.ExtentState.OriginalExtent
QgsExtentWidget.CurrentExtent = QgsExtentWidget.ExtentState.CurrentExtent
QgsExtentWidget.UserExtent = QgsExtentWidget.ExtentState.UserExtent
QgsExtentWidget.ProjectLayerExtent = QgsExtentWidget.ExtentState.ProjectLayerExtent
QgsExtentWidget.DrawOnCanvas = QgsExtentWidget.ExtentState.DrawOnCanvas
QgsExtentWidget.CondensedStyle = QgsExtentWidget.WidgetStyle.CondensedStyle
QgsExtentWidget.ExpandedStyle = QgsExtentWidget.WidgetStyle.ExpandedStyle
try:
    QgsExtentWidget.__attribute_docs__ = {'extentChanged': "Emitted when the widget's extent is changed.\n", 'validationChanged': "Emitted when the widget's validation state changes.\n", 'toggleDialogVisibility': 'Emitted when the parent dialog visibility must be changed (e.g.\nto permit access to the map canvas)\n'}
    QgsExtentWidget.__signal_arguments__ = {'extentChanged': ['r: QgsRectangle'], 'validationChanged': ['valid: bool'], 'toggleDialogVisibility': ['visible: bool']}
except (NameError, AttributeError):
    pass
