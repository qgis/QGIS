# The following has been generated automatically from src/gui/qgscolorwidgets.h
QgsColorWidget.Multiple = QgsColorWidget.ColorComponent.Multiple
QgsColorWidget.Red = QgsColorWidget.ColorComponent.Red
QgsColorWidget.Green = QgsColorWidget.ColorComponent.Green
QgsColorWidget.Blue = QgsColorWidget.ColorComponent.Blue
QgsColorWidget.Hue = QgsColorWidget.ColorComponent.Hue
QgsColorWidget.Saturation = QgsColorWidget.ColorComponent.Saturation
QgsColorWidget.Value = QgsColorWidget.ColorComponent.Value
QgsColorWidget.Alpha = QgsColorWidget.ColorComponent.Alpha
QgsColorWidget.Cyan = QgsColorWidget.ColorComponent.Cyan
QgsColorWidget.Magenta = QgsColorWidget.ColorComponent.Magenta
QgsColorWidget.Yellow = QgsColorWidget.ColorComponent.Yellow
QgsColorWidget.Black = QgsColorWidget.ColorComponent.Black
QgsColorRampWidget.Horizontal = QgsColorRampWidget.Orientation.Horizontal
QgsColorRampWidget.Vertical = QgsColorRampWidget.Orientation.Vertical
QgsColorTextWidget.HexRgb = QgsColorTextWidget.ColorTextFormat.HexRgb
QgsColorTextWidget.HexRgbA = QgsColorTextWidget.ColorTextFormat.HexRgbA
QgsColorTextWidget.Rgb = QgsColorTextWidget.ColorTextFormat.Rgb
QgsColorTextWidget.Rgba = QgsColorTextWidget.ColorTextFormat.Rgba
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
    QgsColorRampWidget.__attribute_docs__ = {'valueChanged': "Emitted when the widget's color component value changes\n\n:param value: new value of color component\n"}
except NameError:
    pass
QgsColorWidget.createDragIcon = staticmethod(QgsColorWidget.createDragIcon)
QgsColorWidget.alterColor = staticmethod(QgsColorWidget.alterColor)
