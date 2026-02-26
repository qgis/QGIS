# The following has been generated automatically from src/core/qgsapplication.h
try:
    QgsApplication.__attribute_docs__ = {'customVariablesChanged': 'Emitted whenever a custom global variable changes.\n', 'nullRepresentationChanged': 'Emitted when the string representing the `NULL` value is changed.\n\n.. seealso:: :py:func:`setNullRepresentation`\n\n.. seealso:: :py:func:`nullRepresentation`\n', 'requestForTranslatableObjects': 'Emitted when project strings which require translation are being\ncollected for inclusion in a .ts file. In order to register translatable\nstrings, connect to this signal and register the strings within the\nspecified \\a translationContext.\n\n.. versionadded:: 3.4\n', 'localeChanged': 'Emitted when project locale has been changed.\n\n.. versionadded:: 3.22.2\n', 'themeChanged': 'Emitted when the application theme has changed.\n\n.. versionadded:: 4.0\n'}
    QgsApplication.__signal_arguments__ = {'requestForTranslatableObjects': ['translationContext: QgsTranslationContext']}
except (NameError, AttributeError):
    pass
