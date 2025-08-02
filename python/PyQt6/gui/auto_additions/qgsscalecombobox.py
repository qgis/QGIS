# The following has been generated automatically from src/gui/qgsscalecombobox.h
# monkey patching scoped based enum
QgsScaleComboBox.RatioMode.ForceUnitNumerator.__doc__ = "Default mode, forces the scale numerator to be 1, e.g. \"1:1000\""
QgsScaleComboBox.RatioMode.Flexible.__doc__ = "Allows numerator values other than 1, e.g: \"2:3\"."
QgsScaleComboBox.RatioMode.__doc__ = """Scale ratio modes.

.. versionadded:: 4.0

* ``ForceUnitNumerator``: Default mode, forces the scale numerator to be 1, e.g. \"1:1000\"
* ``Flexible``: Allows numerator values other than 1, e.g: \"2:3\".

"""
# --
QgsScaleComboBox.RatioMode.baseClass = QgsScaleComboBox
try:
    QgsScaleComboBox.__attribute_docs__ = {'scaleChanged': 'Emitted when *user* has finished editing/selecting a new scale. The\n``scale`` value indicates the scale denominator, e.g. 1000.0 for a\n1:1000 map.\n', 'ratioModeChanged': 'Emitted when the ratio mode for the widget is changed.\n\n.. versionadded:: 4.0\n'}
    QgsScaleComboBox.toString = staticmethod(QgsScaleComboBox.toString)
    QgsScaleComboBox.toDouble = staticmethod(QgsScaleComboBox.toDouble)
    QgsScaleComboBox.__overridden_methods__ = ['showPopup']
    QgsScaleComboBox.__signal_arguments__ = {'scaleChanged': ['scale: float'], 'ratioModeChanged': ['mode: QgsScaleComboBox.RatioMode']}
except (NameError, AttributeError):
    pass
