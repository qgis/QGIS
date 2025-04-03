# The following has been generated automatically from src/gui/qgsextentgroupbox.h
QgsExtentGroupBox.OriginalExtent = QgsExtentGroupBox.ExtentState.OriginalExtent
QgsExtentGroupBox.CurrentExtent = QgsExtentGroupBox.ExtentState.CurrentExtent
QgsExtentGroupBox.UserExtent = QgsExtentGroupBox.ExtentState.UserExtent
QgsExtentGroupBox.ProjectLayerExtent = QgsExtentGroupBox.ExtentState.ProjectLayerExtent
QgsExtentGroupBox.DrawOnCanvas = QgsExtentGroupBox.ExtentState.DrawOnCanvas
try:
    QgsExtentGroupBox.__attribute_docs__ = {'extentChanged': "Emitted when the widget's extent is changed.\n", 'extentLayerChanged': 'Emitted when the extent layer is changed.\n\n.. versionadded:: 3.44\n'}
    QgsExtentGroupBox.__signal_arguments__ = {'extentChanged': ['r: QgsRectangle'], 'extentLayerChanged': ['layer: QgsMapLayer']}
except (NameError, AttributeError):
    pass
