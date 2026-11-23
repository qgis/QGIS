# The following has been generated automatically from src/gui/processing/qgsprocessingmodelerparameterwidget.h
try:
    QgsProcessingModelerParameterWidget.__attribute_docs__ = {'changed': 'Emitted whenever the definition of the parameter is changed in the\nwidget.\n\n.. versionadded:: 4.0\n', 'inputParameterHighlighted': 'Emitted when an input parameter is temporarily highlighted (preselected)\nin the widget.\n\n``parameterName`` will be empty if a previously highlighted parameter\nhas been unhighlighted.\n\n.. seealso:: :py:func:`childAlgorithmOutputHighlighted`\n\n.. versionadded:: 4.6\n', 'childAlgorithmOutputHighlighted': 'Emitted when an child algorithm output is temporarily highlighted\n(preselected) in the widget.\n\n``childId`` will be empty if a previously highlighted output has been\nunhighlighted.\n\n.. seealso:: :py:func:`inputParameterHighlighted`\n\n.. versionadded:: 4.6\n'}
    QgsProcessingModelerParameterWidget.__virtual_methods__ = ['value']
    QgsProcessingModelerParameterWidget.__overridden_methods__ = ['createExpressionContext']
    QgsProcessingModelerParameterWidget.__signal_arguments__ = {'inputParameterHighlighted': ['parameterName: str'], 'childAlgorithmOutputHighlighted': ['childId: str', 'outputName: str']}
    QgsProcessingModelerParameterWidget.__group__ = ['processing']
except (NameError, AttributeError):
    pass
