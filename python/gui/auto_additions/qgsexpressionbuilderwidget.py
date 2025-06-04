# The following has been generated automatically from src/gui/qgsexpressionbuilderwidget.h
QgsExpressionBuilderWidget.Flag.baseClass = QgsExpressionBuilderWidget
Flag = QgsExpressionBuilderWidget  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsExpressionBuilderWidget.__attribute_docs__ = {'expressionParsed': 'Emitted when the user changes the expression in the widget. Users of\nthis widget should connect to this signal to decide if to let the user\ncontinue.\n\n:param isValid: Is ``True`` if the expression the user has typed is\n                valid.\n', 'evalErrorChanged': 'Will be set to ``True`` if the current expression text reported an eval\nerror with the context.\n', 'parserErrorChanged': 'Will be set to ``True`` if the current expression text reported a parser\nerror with the context.\n'}
    QgsExpressionBuilderWidget.__overridden_methods__ = ['showEvent']
    QgsExpressionBuilderWidget.__signal_arguments__ = {'expressionParsed': ['isValid: bool']}
except (NameError, AttributeError):
    pass
