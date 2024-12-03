# The following has been generated automatically from src/gui/editorwidgets/core/qgseditorwidgetwrapper.h
QgsEditorWidgetWrapper.ConstraintResultPass = QgsEditorWidgetWrapper.ConstraintResult.ConstraintResultPass
QgsEditorWidgetWrapper.ConstraintResultFailHard = QgsEditorWidgetWrapper.ConstraintResult.ConstraintResultFailHard
QgsEditorWidgetWrapper.ConstraintResultFailSoft = QgsEditorWidgetWrapper.ConstraintResult.ConstraintResultFailSoft
try:
    QgsEditorWidgetWrapper.__attribute_docs__ = {'valueChanged': 'Emit this signal, whenever the value changed.\n\n:param value: The new value\n\n.. deprecated:: 3.10\n\n   Use valuesChanged signal instead.\n', 'valuesChanged': 'Emit this signal, whenever the value changed.\nIt will also return the values for the additional fields handled by the widget\n\n:param value: The new value\n:param additionalFieldValues: A map of additional field names with their corresponding values\n\n.. versionadded:: 3.10\n', 'constraintStatusChanged': 'Emit this signal when the constraint status changed.\nconstraintStatusChanged\n\n:param constraint: represented as a string\n:param desc: is the constraint description\n:param err: the error represented as a string. Empty if none.\n:param status:\n', 'constraintResultVisibleChanged': 'Emit this signal when the constraint result visibility changed.\n'}
    QgsEditorWidgetWrapper.fromWidget = staticmethod(QgsEditorWidgetWrapper.fromWidget)
    QgsEditorWidgetWrapper.isInTable = staticmethod(QgsEditorWidgetWrapper.isInTable)
    QgsEditorWidgetWrapper.__signal_arguments__ = {'valueChanged': ['value: object'], 'valuesChanged': ['value: object', 'additionalFieldValues: List[object] = []'], 'constraintStatusChanged': ['constraint: str', 'desc: str', 'err: str', 'status: QgsEditorWidgetWrapper.ConstraintResult'], 'constraintResultVisibleChanged': ['visible: bool']}
    QgsEditorWidgetWrapper.__group__ = ['editorwidgets', 'core']
except (NameError, AttributeError):
    pass
