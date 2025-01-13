# The following has been generated automatically from src/gui/editorwidgets/qgsspinbox.h
QgsSpinBox.MinimumValue = QgsSpinBox.ClearValueMode.MinimumValue
QgsSpinBox.MaximumValue = QgsSpinBox.ClearValueMode.MaximumValue
QgsSpinBox.CustomValue = QgsSpinBox.ClearValueMode.CustomValue
try:
    QgsSpinBox.__attribute_docs__ = {'returnPressed': 'Emitted when the Return or Enter key is used in the line edit\n\n.. versionadded:: 3.40\n', 'textEdited': 'Emitted when the the value has been manually edited via line edit.\n\n.. versionadded:: 3.40\n', 'editingTimeout': 'Emitted when either:\n\n1. 1 second has elapsed since the last value change in the widget (eg last key press or scroll wheel event)\n2. or, immediately after the widget has lost focus after its value was changed.\n\nThis signal can be used to respond semi-instantly to changes in the spin box, without responding too quickly\nwhile the user in the middle of setting the value.\n\n.. seealso:: :py:func:`editingTimeoutInterval`\n\n.. versionadded:: 3.42\n'}
    QgsSpinBox.__signal_arguments__ = {'textEdited': ['text: str'], 'editingTimeout': ['value: int']}
    QgsSpinBox.__group__ = ['editorwidgets']
except (NameError, AttributeError):
    pass
