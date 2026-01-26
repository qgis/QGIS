# The following has been generated automatically from src/gui/editorwidgets/core/qgssearchwidgetwrapper.h
try:
    QgsSearchWidgetWrapper.__attribute_docs__ = {'expressionChanged': 'Emitted whenever the expression changes\n\n:param exp: The new search expression\n', 'valueChanged': 'Emitted when a user changes the value of the search widget.\n', 'valueCleared': 'Emitted when a user changes the value of the search widget back to an\nempty, default state.\n'}
    QgsSearchWidgetWrapper.exclusiveFilterFlags = staticmethod(QgsSearchWidgetWrapper.exclusiveFilterFlags)
    QgsSearchWidgetWrapper.nonExclusiveFilterFlags = staticmethod(QgsSearchWidgetWrapper.nonExclusiveFilterFlags)
    QgsSearchWidgetWrapper.toString = staticmethod(QgsSearchWidgetWrapper.toString)
    QgsSearchWidgetWrapper.__virtual_methods__ = ['supportedFlags', 'defaultFlags', 'createExpression', 'clearWidget']
    QgsSearchWidgetWrapper.__abstract_methods__ = ['expression', 'applyDirectly', 'setExpression']
    QgsSearchWidgetWrapper.__overridden_methods__ = ['setEnabled', 'setFeature']
    QgsSearchWidgetWrapper.__signal_arguments__ = {'expressionChanged': ['exp: str']}
    QgsSearchWidgetWrapper.__group__ = ['editorwidgets', 'core']
except (NameError, AttributeError):
    pass
