# The following has been generated automatically from src/gui/qgsattributeformeditorwidget.h
try:
    QgsAttributeFormEditorWidget.__attribute_docs__ = {'valueChanged': "Emitted when the widget's value changes\n\n:param value: new widget value\n\n.. deprecated:: 3.10\n\n   Use :py:func:`~QgsAttributeFormEditorWidget.valuesChanged` instead.\n", 'valuesChanged': "Emitted when the widget's value changes\n\n:param value: new widget value\n:param additionalFieldValues: of the potential additional fields\n\n.. versionadded:: 3.10\n", 'rememberLastValueChanged': "Emitted when the widget's remember last value toggle changes\n\n:param index: the field index\n:param remember: the value is ``True`` when the last value should be\n                 remembered\n\n.. versionadded:: 4.0\n"}
    QgsAttributeFormEditorWidget.__overridden_methods__ = ['createSearchWidgetWrappers']
    QgsAttributeFormEditorWidget.__signal_arguments__ = {'valueChanged': ['value: object'], 'valuesChanged': ['value: object', 'additionalFieldValues: List[object]'], 'rememberLastValueChanged': ['index: int', 'remember: bool']}
except (NameError, AttributeError):
    pass
