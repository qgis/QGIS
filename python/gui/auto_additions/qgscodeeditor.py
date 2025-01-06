# The following has been generated automatically from src/gui/codeeditors/qgscodeeditor.h
# monkey patching scoped based enum
QgsCodeEditor.Mode.ScriptEditor.__doc__ = "Standard mode, allows for display and edit of entire scripts"
QgsCodeEditor.Mode.OutputDisplay.__doc__ = "Read only mode for display of command outputs"
QgsCodeEditor.Mode.CommandInput.__doc__ = "Command input mode"
QgsCodeEditor.Mode.__doc__ = """Code editor modes.

.. versionadded:: 3.30

* ``ScriptEditor``: Standard mode, allows for display and edit of entire scripts
* ``OutputDisplay``: Read only mode for display of command outputs
* ``CommandInput``: Command input mode

"""
# --
QgsCodeEditor.Mode.baseClass = QgsCodeEditor
# monkey patching scoped based enum
QgsCodeEditor.LineNumbers = QgsCodeEditor.MarginRole.LineNumbers
QgsCodeEditor.LineNumbers.is_monkey_patched = True
QgsCodeEditor.LineNumbers.__doc__ = "Line numbers"
QgsCodeEditor.ErrorIndicators = QgsCodeEditor.MarginRole.ErrorIndicators
QgsCodeEditor.ErrorIndicators.is_monkey_patched = True
QgsCodeEditor.ErrorIndicators.__doc__ = "Error indicators"
QgsCodeEditor.FoldingControls = QgsCodeEditor.MarginRole.FoldingControls
QgsCodeEditor.FoldingControls.is_monkey_patched = True
QgsCodeEditor.FoldingControls.__doc__ = "Folding controls"
QgsCodeEditor.MarginRole.__doc__ = """Margin roles.

This enum contains the roles which the different numbered margins are used for.

.. versionadded:: 3.16

* ``LineNumbers``: Line numbers
* ``ErrorIndicators``: Error indicators
* ``FoldingControls``: Folding controls

"""
# --
QgsCodeEditor.MarginRole.baseClass = QgsCodeEditor
# monkey patching scoped based enum
QgsCodeEditor.Flag.CodeFolding.__doc__ = "Indicates that code folding should be enabled for the editor"
QgsCodeEditor.Flag.ImmediatelyUpdateHistory.__doc__ = "Indicates that the history file should be immediately updated whenever a command is executed, instead of the default behavior of only writing the history on widget close \n.. versionadded:: 3.32"
QgsCodeEditor.Flag.__doc__ = """Flags controlling behavior of code editor

.. versionadded:: 3.28

* ``CodeFolding``: Indicates that code folding should be enabled for the editor
* ``ImmediatelyUpdateHistory``: Indicates that the history file should be immediately updated whenever a command is executed, instead of the default behavior of only writing the history on widget close

  .. versionadded:: 3.32


"""
# --
QgsCodeEditor.Flag.baseClass = QgsCodeEditor
QgsCodeEditor.Flags.baseClass = QgsCodeEditor
Flags = QgsCodeEditor  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsCodeEditor.__attribute_docs__ = {'SEARCH_RESULT_INDICATOR': 'Indicator index for search results', 'sessionHistoryCleared': 'Emitted when the history of commands run in the current session is cleared.\n\n.. versionadded:: 3.30\n', 'persistentHistoryCleared': 'Emitted when the persistent history of commands run in the editor is cleared.\n\n.. versionadded:: 3.30\n', 'helpRequested': 'Emitted when documentation was requested for the specified ``word``.\n\n.. versionadded:: 3.42\n', 'editingTimeout': 'Emitted when either:\n\n1. 1 second has elapsed since the last text change in the widget\n2. or, immediately after the widget has lost focus after its text was changed.\n\n.. seealso:: :py:func:`editingTimeoutInterval`\n\n.. versionadded:: 3.42\n'}
    QgsCodeEditor.languageToString = staticmethod(QgsCodeEditor.languageToString)
    QgsCodeEditor.defaultColor = staticmethod(QgsCodeEditor.defaultColor)
    QgsCodeEditor.color = staticmethod(QgsCodeEditor.color)
    QgsCodeEditor.setColor = staticmethod(QgsCodeEditor.setColor)
    QgsCodeEditor.getMonospaceFont = staticmethod(QgsCodeEditor.getMonospaceFont)
    QgsCodeEditor.isFixedPitch = staticmethod(QgsCodeEditor.isFixedPitch)
    QgsCodeEditor.__signal_arguments__ = {'helpRequested': ['word: str']}
    QgsCodeEditor.__group__ = ['codeeditors']
except (NameError, AttributeError):
    pass
try:
    QgsCodeInterpreter.__group__ = ['codeeditors']
except (NameError, AttributeError):
    pass
