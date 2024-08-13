# The following has been generated automatically from src/gui/codeeditors/qgscodeeditor.h
# monkey patching scoped based enum
QgsCodeEditor.Mode.ScriptEditor.__doc__ = "Standard mode, allows for display and edit of entire scripts"
QgsCodeEditor.Mode.OutputDisplay.__doc__ = "Read only mode for display of command outputs"
QgsCodeEditor.Mode.CommandInput.__doc__ = "Command input mode"
QgsCodeEditor.Mode.__doc__ = "Code editor modes.\n\n.. versionadded:: 3.30\n\n" + '* ``ScriptEditor``: ' + QgsCodeEditor.Mode.ScriptEditor.__doc__ + '\n' + '* ``OutputDisplay``: ' + QgsCodeEditor.Mode.OutputDisplay.__doc__ + '\n' + '* ``CommandInput``: ' + QgsCodeEditor.Mode.CommandInput.__doc__
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
QgsCodeEditor.MarginRole.__doc__ = "Margin roles.\n\nThis enum contains the roles which the different numbered margins are used for.\n\n.. versionadded:: 3.16\n\n" + '* ``LineNumbers``: ' + QgsCodeEditor.MarginRole.LineNumbers.__doc__ + '\n' + '* ``ErrorIndicators``: ' + QgsCodeEditor.MarginRole.ErrorIndicators.__doc__ + '\n' + '* ``FoldingControls``: ' + QgsCodeEditor.MarginRole.FoldingControls.__doc__
# --
QgsCodeEditor.MarginRole.baseClass = QgsCodeEditor
# monkey patching scoped based enum
QgsCodeEditor.Flag.CodeFolding.__doc__ = "Indicates that code folding should be enabled for the editor"
QgsCodeEditor.Flag.ImmediatelyUpdateHistory.__doc__ = "Indicates that the history file should be immediately updated whenever a command is executed, instead of the default behavior of only writing the history on widget close. Since QGIS 3.32."
QgsCodeEditor.Flag.__doc__ = "Flags controlling behavior of code editor\n\n.. versionadded:: 3.28\n\n" + '* ``CodeFolding``: ' + QgsCodeEditor.Flag.CodeFolding.__doc__ + '\n' + '* ``ImmediatelyUpdateHistory``: ' + QgsCodeEditor.Flag.ImmediatelyUpdateHistory.__doc__
# --
QgsCodeEditor.Flag.baseClass = QgsCodeEditor
QgsCodeEditor.Flags.baseClass = QgsCodeEditor
Flags = QgsCodeEditor  # dirty hack since SIP seems to introduce the flags in module
QgsCodeEditor.__attribute_docs__ = {'SEARCH_RESULT_INDICATOR': 'Indicator index for search results', 'sessionHistoryCleared': 'Emitted when the history of commands run in the current session is cleared.\n\n.. versionadded:: 3.30\n', 'persistentHistoryCleared': 'Emitted when the persistent history of commands run in the editor is cleared.\n\n.. versionadded:: 3.30\n'}
