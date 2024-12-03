# The following has been generated automatically from src/core/qgsfeedback.h
try:
    QgsFeedback.__attribute_docs__ = {'canceled': 'Internal routines can connect to this signal if they use event loop\n', 'progressChanged': 'Emitted when the feedback object reports a progress change. Depending on how the\nfeedback object is used progress reporting may not be supported. The ``progress``\nargument is in percentage and ranges from 0-100.\n\n.. seealso:: :py:func:`setProgress`\n\n.. seealso:: :py:func:`progress`\n', 'processedCountChanged': 'Emitted when the feedback object reports a change in the number of processed objects.\nDepending on how the feedback object is used processed count reporting may not be supported. The ``processedCount``\nargument is an unsigned long integer and starts from 0.\n\n.. seealso:: :py:func:`setProgress`\n\n.. seealso:: :py:func:`progress`\n\n.. versionadded:: 3.24\n'}
    QgsFeedback.__signal_arguments__ = {'progressChanged': ['progress: float'], 'processedCountChanged': ['processedCount: int']}
except (NameError, AttributeError):
    pass
