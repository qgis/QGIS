# The following has been generated automatically from src/core/processing/qgsprocessingfeedback.h
try:
    QgsProcessingFeedback.__attribute_docs__ = {'progressTextChanged': 'Raised when the progress ``text`` is changed.\n\n.. seealso:: :py:func:`setProgressText`\n\n.. versionadded:: 4.2\n', 'errorReported': 'Raised when an error is reported.\n\n.. seealso:: :py:func:`reportError`\n\n.. versionadded:: 4.2\n', 'warningPushed': 'Raised when an warning is pushed.\n\n.. seealso:: :py:func:`pushWarning`\n\n.. versionadded:: 4.2\n', 'infoPushed': 'Raised when information ``text`` is pushed.\n\n.. seealso:: :py:func:`pushInfo`\n\n.. versionadded:: 4.2\n', 'commandInfoPushed': 'Raised when command information ``text`` is pushed.\n\n.. seealso:: :py:func:`pushCommandInfo`\n\n.. versionadded:: 4.2\n', 'debugInfoPushed': 'Raised when debug information ``text`` is pushed.\n\n.. seealso:: :py:func:`pushDebugInfo`\n\n.. versionadded:: 4.2\n', 'consoleInfoPushed': 'Raised when console information ``text`` is pushed.\n\n.. seealso:: :py:func:`pushConsoleInfo`\n\n.. versionadded:: 4.2\n', 'formattedMessagePushed': 'Raised when a formatted ``html`` message is pushed.\n\n.. seealso:: :py:func:`pushFormattedMessage`\n\n.. versionadded:: 4.2\n'}
    QgsProcessingFeedback.__virtual_methods__ = ['setProgressText', 'reportError', 'pushWarning', 'pushInfo', 'pushFormattedMessage', 'pushCommandInfo', 'pushDebugInfo', 'pushConsoleInfo', 'htmlLog', 'textLog']
    QgsProcessingFeedback.__signal_arguments__ = {'progressTextChanged': ['text: str'], 'errorReported': ['text: str', 'fatalError: bool'], 'warningPushed': ['text: str'], 'infoPushed': ['text: str'], 'commandInfoPushed': ['text: str'], 'debugInfoPushed': ['text: str'], 'consoleInfoPushed': ['text: str'], 'formattedMessagePushed': ['html: str']}
    QgsProcessingFeedback.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingMultiStepFeedback.__overridden_methods__ = ['setProgressText', 'reportError', 'pushWarning', 'pushInfo', 'pushCommandInfo', 'pushDebugInfo', 'pushConsoleInfo', 'pushFormattedMessage', 'htmlLog', 'textLog']
    QgsProcessingMultiStepFeedback.__group__ = ['processing']
except (NameError, AttributeError):
    pass
