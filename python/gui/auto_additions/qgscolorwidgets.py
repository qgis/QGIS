# The following has been generated automatically from src/gui/qgscolorwidgets.h
QgsColorTextWidget.ColorTextFormat.baseClass = QgsColorTextWidget
try:
    QgsColorWidget.__attribute_docs__ = {'colorChanged': "Emitted when the widget's color changes\n\n:param color: new widget color\n", 'hovered': 'Emitted when mouse hovers over widget.\n'}
except NameError:
    pass
try:
    QgsColorWidgetAction.__attribute_docs__ = {'colorChanged': 'Emitted when a color has been selected from the widget\n\n:param color: selected color\n'}
except NameError:
    pass
try:
    QgsColorRampWidget.__attribute_docs__ = {'valueChanged': "Emitted when the widget's color component value changes\n\n:param value: new value of color component in the range between 0 and the value returned by :py:func:`~QgsColorRampWidget.componentRange`\n\n.. deprecated:: QGIS 3.40.\n   Use :py:func:`~QgsColorRampWidget.valueChangedF` instead.\n", 'valueChangedF': "Emitted when the widget's color component value changes\n\n:param value: new value of color component in the range 0.0-1.0\n\n.. versionadded:: 3.40\n"}
except NameError:
    pass
QgsColorWidget.createDragIcon = staticmethod(QgsColorWidget.createDragIcon)
QgsColorWidget.alterColor = staticmethod(QgsColorWidget.alterColor)
QgsColorWidget.alterColorF = staticmethod(QgsColorWidget.alterColorF)
