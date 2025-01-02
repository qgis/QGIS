# The following has been generated automatically from src/gui/qgsfilterlineedit.h
QgsFilterLineEdit.ClearToNull = QgsFilterLineEdit.ClearMode.ClearToNull
QgsFilterLineEdit.ClearToDefault = QgsFilterLineEdit.ClearMode.ClearToDefault
QgsFilterLineEdit.ClearMode.baseClass = QgsFilterLineEdit
try:
    QgsFilterLineEdit.__attribute_docs__ = {'cleared': 'Emitted when the widget is cleared\n\n.. seealso:: :py:func:`clearValue`\n', 'valueChanged': 'Same as :py:func:`~QgsFilterLineEdit.textChanged` but with support for null values.\n\n:param value: The current text or null string if it matches the :py:func:`~QgsFilterLineEdit.nullValue` property.\n', 'showSpinnerChanged': 'Show a spinner icon. This can be used for search boxes to indicate that\nsomething is going on in the background.\n', 'selectOnFocusChanged': 'Will select all text when this widget receives the focus.\n'}
    QgsFilterLineEdit.__signal_arguments__ = {'valueChanged': ['value: str']}
except (NameError, AttributeError):
    pass
