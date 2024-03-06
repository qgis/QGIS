# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2012-09-10
copyright            : (C) 2012 by Salvatore Larosa
email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Some portions of code were taken from https://code.google.com/p/pydee/
"""

import code
import os
import re
import sys
import traceback
from pathlib import Path
from tempfile import NamedTemporaryFile

from qgis.PyQt.Qsci import QsciScintilla
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtGui import QKeySequence, QFontMetrics, QClipboard
from qgis.PyQt.QtWidgets import QShortcut, QApplication
from qgis.core import QgsApplication, QgsSettings, Qgis
from qgis.gui import (
    QgsCodeEditorPython,
    QgsCodeEditorColorScheme,
    QgsCodeEditor,
    QgsCodeInterpreter
)

from .process_wrapper import ProcessWrapper

_init_statements = [
    # Python
    "import sys",
    "import os",
    "from pathlib import Path",
    "import re",
    "import math",
    # QGIS
    "from qgis.core import *",
    "from qgis.gui import *",
    "from qgis.analysis import *",
    # # 3D might not be compiled in
    """
try:
    from qgis._3d import *
except ModuleNotFoundError:
    pass
""",
    "import processing",
    "import qgis.utils",
    "from qgis.utils import iface",
    # Qt
    "from qgis.PyQt.QtCore import *",
    "from qgis.PyQt.QtGui import *",
    "from qgis.PyQt.QtWidgets import *",
    "from qgis.PyQt.QtNetwork import *",
    "from qgis.PyQt.QtXml import *",

    r"""
def __parse_object(object=None):
    if not object:
        return None
    import inspect
    if inspect.isclass(object):
        str_class = str(object)
    else:
        str_class = str(object.__class__)

    qgis_api_pattern = r".*qgis\._(\w+)\.(\w+).*"
    match = re.match(qgis_api_pattern, str_class)
    if match:
        module = match[1]
        obj = match[2]
        return 'qgis', module, obj

    pyqt_pattern = r".*PyQt5\.(\w+)\.(\w+).*"
    match = re.match(pyqt_pattern, str_class)
    if match:
        module = match[1]
        obj = match[2]
        return 'qt', module, obj
""",
    r"""
def _api(object=None):
    '''
    Link to the QGIS API documentation for the given object.
    If no object is given, the main API page is opened.
    If the object is not part of the QGIS API but is a Qt object the Qt documentation is opened.
    '''
    import webbrowser
    api = __parse_object(object)

    version = '' if 'master' in Qgis.QGIS_VERSION.lower() else re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]

    if not api:
        webbrowser.open(f"https://qgis.org/api/{version}")
    elif api[0] == 'qgis':
        webbrowser.open(f"https://api.qgis.org/api/{version}/class{api[2]}.html")
    elif api[0] == 'qt':
        qtversion = '.'.join(qVersion().split(".")[:2])
        webbrowser.open(f"https://doc.qt.io/qt-{qtversion}/{api[2].lower()}.html")
""",
    r"""
def _pyqgis(object=None):
    '''
    Link to the PyQGIS API documentation for the given object.
    If no object is given, the main PyQGIS API page is opened.
    If the object is not part of the QGIS API but is a Qt object the Qt documentation is opened.
    '''
    import webbrowser
    api = __parse_object(object)

    version = 'master' if 'master' in Qgis.QGIS_VERSION.lower() else re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]

    if not api:
        webbrowser.open(f"https://qgis.org/pyqgis/{version}")
    elif api[0] == 'qgis':
        webbrowser.open(f"https://qgis.org/pyqgis/{version}/{api[1]}/{api[2]}.html")
    elif api[0] == 'qt':
        qtversion = '.'.join(qVersion().split(".")[:2])
        webbrowser.open(f"https://doc.qt.io/qt-{qtversion}/{api[2].lower()}.html")
"""
]


# States of the interpreter
PS1 = 0  # Writing a new command
PS2 = 1  # Continuation of a multi-line command
SUBPROCESS = 2  # Sending input to a subprocess


class PythonInterpreter(QgsCodeInterpreter, code.InteractiveInterpreter):

    def __init__(self, shell):
        super(QgsCodeInterpreter, self).__init__()
        code.InteractiveInterpreter.__init__(self, locals=None)

        self.shell = shell
        self.sub_process = None
        self.buffer = []

        for statement in _init_statements:
            try:
                self.runsource(statement)
            except ModuleNotFoundError:
                pass

    def execCommandImpl(self, cmd, show_input=True):

        # Child process running, input should be sent to it
        if self.currentState() == SUBPROCESS:
            sys.stdout.write(cmd + "\n")
            self.sub_process.write(cmd)
            return 0

        if show_input:
            self.writeCMD(cmd)

        if self.currentState() == PS1:

            # This line makes single line commands with leading spaces work
            cmd = cmd.strip()

            # User entered: varname = !cmd
            # Run the command and assign the output to varname
            # Mimics IPython's behavior
            assignment_pattern = r"(\w+)\s*=\s*!(.+)"
            match = re.match(assignment_pattern, cmd)
            if match:
                varname = match[1]
                cmd = match[2]
                # Run the command in non-interactive mode
                self.sub_process = ProcessWrapper(cmd, interactive=False)
                # Concatenate stdout and stderr
                res = (self.sub_process.stdout + self.sub_process.stderr).strip()

                # Use a temporary file to communicate the result to the inner interpreter
                tmp = Path(NamedTemporaryFile(delete=False).name)
                tmp.write_text(res, encoding="utf-8")
                self.runsource(f'{varname} = Path("{tmp}").read_text(encoding="utf-8").split("\\n")')
                tmp.unlink()
                self.sub_process = None
                return 0

            # User entered: !cmd
            # Run the command and stream the output to the console
            # While the process is running, the console is in state 2 meaning
            # that all input is sent to the child process
            # Mimics IPython's behavior
            elif cmd.startswith("!"):
                cmd = cmd[1:]
                self.sub_process = ProcessWrapper(cmd)
                self.sub_process.finished.connect(self.processFinished)
                return 0

        res = 0

        import webbrowser
        version = 'master' if 'master' in Qgis.QGIS_VERSION.lower() else \
            re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]

        if cmd == "?":
            self.shell.parent.shellOut.insertHelp()
        elif cmd == '_pyqgis':
            webbrowser.open("https://qgis.org/pyqgis/{}".format(version))
        elif cmd == '_api':
            webbrowser.open("https://qgis.org/api/{}".format('' if version == 'master' else version))
        elif cmd == '_cookbook':
            webbrowser.open(
                "https://docs.qgis.org/{}/en/docs/pyqgis_developer_cookbook/".format(
                    'testing' if version == 'master' else version))
        else:
            self.buffer.append(cmd)
            src = "\n".join(self.buffer)
            res = self.runsource(src)
            if res == 0:
                self.buffer = []

        return res

    def writeCMD(self, txt):
        if sys.stdout:
            sys.stdout.fire_keyboard_interrupt = False
        if len(txt) > 0:
            sys.stdout.write(f'{self.promptForState()} {txt}\n')

    def runsource(self, source, filename='<input>', symbol='single'):
        if sys.stdout:
            sys.stdout.fire_keyboard_interrupt = False

        hook = sys.excepthook
        try:
            def excepthook(etype, value, tb):
                self.write("".join(traceback.format_exception(etype, value, tb)))

            sys.excepthook = excepthook

            return super(PythonInterpreter, self).runsource(source, filename, symbol)
        finally:
            sys.excepthook = hook

    def currentState(self):
        if self.sub_process:
            return SUBPROCESS
        return super().currentState()

    def promptForState(self, state=-1):
        if state == -1:
            state = self.currentState()
        if state == SUBPROCESS:
            return " : "
        elif state == PS2:
            return "..."
        else:
            return ">>>"

    def processFinished(self, errorcode):
        self.sub_process = None
        self.shell.updatePrompt()


class ShellScintilla(QgsCodeEditorPython):

    def __init__(self, parent=None):
        # We set the ImmediatelyUpdateHistory flag here, as users can easily
        # crash QGIS by entering a Python command, and we don't want the
        # history leading to the crash lost..
        super().__init__(parent, [], QgsCodeEditor.Mode.CommandInput,
                         flags=QgsCodeEditor.Flags(QgsCodeEditor.Flag.CodeFolding | QgsCodeEditor.Flag.ImmediatelyUpdateHistory))

        self.parent = parent
        self._interpreter = PythonInterpreter(self)
        self.setInterpreter(self._interpreter)

        self.opening = ['(', '{', '[', "'", '"']
        self.closing = [')', '}', ']', "'", '"']

        self.setHistoryFilePath(
            os.path.join(QgsApplication.qgisSettingsDirPath(), "console_history.txt"))

        self.refreshSettingsShell()

        # Disable command key
        ctrl, shift = self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Z') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Y') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl + shift)

        # New QShortcut = ctrl+space/ctrl+alt+space for Autocomplete
        self.newShortcutCSS = QShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Modifier.SHIFT | Qt.Key.Key_Space), self)
        self.newShortcutCAS = QShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Modifier.ALT | Qt.Key.Key_Space), self)
        self.newShortcutCSS.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.newShortcutCAS.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.newShortcutCAS.activated.connect(self.autoComplete)
        self.newShortcutCSS.activated.connect(self.showHistory)

        self.sessionHistoryCleared.connect(self.on_session_history_cleared)
        self.persistentHistoryCleared.connect(self.on_persistent_history_cleared)

    def _setMinimumHeight(self):
        font = self.lexer().defaultFont(0)
        fm = QFontMetrics(font)

        self.setMinimumHeight(fm.height() + 10)

    def refreshSettingsShell(self):
        # Set Python lexer
        self.initializeLexer()

        # Sets minimum height for input area based of font metric
        self._setMinimumHeight()

    def on_session_history_cleared(self):
        msgText = QCoreApplication.translate('PythonConsole',
                                             'Session history cleared successfully.')
        self.parent.callWidgetMessageBar(msgText)

    def on_persistent_history_cleared(self):
        msgText = QCoreApplication.translate('PythonConsole',
                                             'History cleared successfully.')
        self.parent.callWidgetMessageBar(msgText)

    def keyPressEvent(self, e):

        if e.modifiers() & (Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.MetaModifier) and e.key() == Qt.Key.Key_C and not self.hasSelectedText():
            if self._interpreter.sub_process:
                sys.stderr.write("Terminate child process\n")
                self._interpreter.sub_process.kill()
                self._interpreter.sub_process = None
                self.updatePrompt()
            return

        # update the live history
        self.updateSoftHistory()
        super().keyPressEvent(e)
        self.updatePrompt()

    def mousePressEvent(self, e):
        """
        Re-implemented to handle the mouse press event.
        e: the mouse press event (QMouseEvent)
        """
        self.setFocus()
        if e.button() == Qt.MouseButton.MiddleButton:
            stringSel = QApplication.clipboard().text(QClipboard.Mode.Selection)
            if not self.isCursorOnLastLine():
                self.moveCursorToEnd()
            self.insertFromDropPaste(stringSel)
            e.accept()
        else:
            QgsCodeEditorPython.mousePressEvent(self, e)

    def paste(self):
        """
        Method to display data from the clipboard.

        XXX: It should reimplement the virtual QScintilla.paste method,
        but it seems not used by QScintilla code.
        """
        stringPaste = QApplication.clipboard().text()
        if self.isCursorOnLastLine():
            if self.hasSelectedText():
                self.removeSelectedText()
        else:
            self.moveCursorToEnd()
        self.insertFromDropPaste(stringPaste)

    # Drag and drop
    def dropEvent(self, e):
        if e.mimeData().hasText():
            stringDrag = e.mimeData().text()
            self.insertFromDropPaste(stringDrag)
            self.setFocus()
            e.setDropAction(Qt.DropAction.CopyAction)
            e.accept()
        else:
            QgsCodeEditorPython.dropEvent(self, e)

    def insertFromDropPaste(self, textDP):
        pasteList = textDP.splitlines()
        if pasteList:
            for line in pasteList[:-1]:
                cleanLine = line.replace(">>> ", "").replace("... ", "")
                self.insert(cleanLine)
                self.moveCursorToEnd()
                self.runCommand(self.text())
            if pasteList[-1] != "":
                line = pasteList[-1]
                cleanLine = line.replace(">>> ", "").replace("... ", "")
                curpos = self.getCursorPosition()
                self.insert(cleanLine)
                self.setCursorPosition(curpos[0], curpos[1] + len(cleanLine))

    def insertTextFromFile(self, listOpenFile):
        for line in listOpenFile[:-1]:
            self.append(line)
            self.moveCursorToEnd()
            self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            self.runCommand(self.text())
        self.append(listOpenFile[-1])
        self.moveCursorToEnd()
        self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def entered(self):
        self.moveCursorToEnd()
        self.runCommand(self.text())
        self.setFocus()
        self.moveCursorToEnd()

    def write(self, txt):
        if sys.stderr:
            sys.stderr.write(txt)

    def runFile(self, filename):
        filename = filename.replace("\\", "/")
        dirname = os.path.dirname(filename)

        # Append the directory of the file to the path and set __file__ to the filename
        self._interpreter.execCommandImpl("sys.path.append('{0}')".format(dirname), False)
        self._interpreter.execCommandImpl("__file__ = '{0}'".format(filename), False)

        try:
            # Run the file
            self.runCommand("exec(Path('{0}').read_text())".format(filename), skipHistory=True)
        finally:
            # Remove the directory from the path and delete the __file__ variable
            self._interpreter.execCommandImpl("del __file__", False)
            self._interpreter.execCommandImpl("sys.path.remove('{0}')".format(dirname), False)
