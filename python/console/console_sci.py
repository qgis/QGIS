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

from qgis.PyQt.Qsci import QsciScintilla
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtGui import QKeySequence, QFontMetrics, QClipboard
from qgis.PyQt.QtWidgets import QShortcut, QApplication
from qgis.core import QgsApplication, QgsSettings, Qgis
from qgis.gui import (
    QgsCodeEditorPython,
    QgsCodeEditorColorScheme,
    QgsCodeEditor
)

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
]


class ShellScintilla(QgsCodeEditorPython, code.InteractiveInterpreter):

    def __init__(self, parent=None):
        super(QgsCodeEditorPython, self).__init__(parent, [], QgsCodeEditor.Mode.CommandInput)
        code.InteractiveInterpreter.__init__(self, locals=None)

        self.parent = parent

        self.opening = ['(', '{', '[', "'", '"']
        self.closing = [')', '}', ']', "'", '"']

        self.settings = QgsSettings()

        self.new_input_line = True

        self.buffer = []
        self.continuationLine = False

        self.displayPrompt(self.continuationLine)

        for statement in _init_statements:
            try:
                self.runsource(statement)
            except ModuleNotFoundError:
                pass

        self.setHistoryFilePath(
            os.path.join(QgsApplication.qgisSettingsDirPath(), "console_history.txt"))

        self.historyDlg = None  # HistoryDialog(self)

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
        self.newShortcutCSS = QShortcut(QKeySequence(Qt.CTRL + Qt.SHIFT + Qt.Key_Space), self)
        self.newShortcutCAS = QShortcut(QKeySequence(Qt.CTRL + Qt.ALT + Qt.Key_Space), self)
        self.newShortcutCSS.setContext(Qt.WidgetShortcut)
        self.newShortcutCAS.setContext(Qt.WidgetShortcut)
        self.newShortcutCAS.activated.connect(self.autoComplete)
        self.newShortcutCSS.activated.connect(self.showHistory)

        self.sessionHistoryCleared.connect(self.on_session_history_cleared)
        self.persistentHistoryCleared.connect(self.on_persistent_history_cleared)

    def initializeLexer(self):
        super().initializeLexer()
        self.setCaretLineVisible(False)
        self.setLineNumbersVisible(False)  # NO linenumbers for the input line
        self.setFoldingVisible(False)
        # Margin 1 is used for the '>>>' prompt (console input)
        self.setMarginLineNumbers(1, True)
        self.setMarginWidth(1, "00000")
        self.setMarginType(1, 5)  # TextMarginRightJustified=5
        self.setMarginsBackgroundColor(self.color(QgsCodeEditorColorScheme.ColorRole.Background))
        self.setEdgeMode(QsciScintilla.EdgeNone)

    def _setMinimumHeight(self):
        font = self.lexer().defaultFont(0)
        fm = QFontMetrics(font)

        self.setMinimumHeight(fm.height() + 10)

    def refreshSettingsShell(self):
        # Set Python lexer
        self.initializeLexer()

        # Sets minimum height for input area based of font metric
        self._setMinimumHeight()

    def moveCursorToStart(self):
        super().moveCursorToStart()
        self.displayPrompt(self.continuationLine)

    def moveCursorToEnd(self):
        super().moveCursorToEnd()
        self.displayPrompt(self.continuationLine)

    def displayPrompt(self, more=False):
        self.SendScintilla(QsciScintilla.SCI_MARGINSETTEXT, 0,
                           str.encode("..." if more else ">>>"))

    def on_session_history_cleared(self):
        msgText = QCoreApplication.translate('PythonConsole',
                                             'Session history cleared successfully.')
        self.parent.callWidgetMessageBar(msgText)

    def on_persistent_history_cleared(self):
        msgText = QCoreApplication.translate('PythonConsole',
                                             'History cleared successfully.')
        self.parent.callWidgetMessageBar(msgText)

    def keyPressEvent(self, e):
        # update the live history
        self.updateSoftHistory()

        startLine, startPos, endLine, endPos = self.getSelection()

        # handle invalid cursor position and multiline selections
        if startLine < endLine:
            # allow copying and selecting
            if e.modifiers() & (Qt.ControlModifier | Qt.MetaModifier):
                if e.key() == Qt.Key_C:
                    # only catch and return from Ctrl-C here if there's a selection
                    if self.hasSelectedText():
                        QsciScintilla.keyPressEvent(self, e)
                        return
                elif e.key() == Qt.Key_A:
                    QsciScintilla.keyPressEvent(self, e)
                    return
                else:
                    return
            # allow selection
            if e.modifiers() & Qt.ShiftModifier:
                if e.key() in (Qt.Key_Left, Qt.Key_Right, Qt.Key_Home, Qt.Key_End):
                    QsciScintilla.keyPressEvent(self, e)
                return
            # all other keystrokes get sent to the input line
            self.moveCursorToEnd()

        if e.modifiers() & (
                Qt.ControlModifier | Qt.MetaModifier) and e.key() == Qt.Key_C and not self.hasSelectedText():
            # keyboard interrupt
            sys.stdout.fire_keyboard_interrupt = True
            return

        line, index = self.getCursorPosition()
        cmd = self.text(line)
        hasSelectedText = self.hasSelectedText()

        if e.key() in (Qt.Key_Return, Qt.Key_Enter) and not self.isListActive():
            self.entered()

        elif e.key() in (Qt.Key_Left, Qt.Key_Home):
            QsciScintilla.keyPressEvent(self, e)

        elif e.key() in (Qt.Key_Backspace, Qt.Key_Delete):
            QsciScintilla.keyPressEvent(self, e)
            self.recolor()

        elif (e.modifiers() & (Qt.ControlModifier | Qt.MetaModifier) and e.key() == Qt.Key_V) or \
                (e.modifiers() & Qt.ShiftModifier and e.key() == Qt.Key_Insert):
            self.paste()
            e.accept()

        elif e.key() == Qt.Key_Down and not self.isListActive():
            self.showPreviousCommand()

        elif e.key() == Qt.Key_Up and not self.isListActive():
            self.showNextCommand()

        # TODO: press event for auto-completion file directory
        else:
            t = e.text()
            self.autoCloseBracket = self.settings.value("pythonConsole/autoCloseBracket", False,
                                                        type=bool)
            self.autoImport = self.settings.value("pythonConsole/autoInsertionImport", True,
                                                  type=bool)
            # Close bracket automatically
            if t in self.opening and self.autoCloseBracket:
                i = self.opening.index(t)
                if self.hasSelectedText() and startPos != 0:
                    selText = self.selectedText()
                    self.removeSelectedText()
                    self.insert(self.opening[i] + selText + self.closing[i])
                    self.setCursorPosition(endLine, endPos + 2)
                    return
                elif t == '(' and (re.match(r'^[ \t]*def \w+$', cmd)
                                   or re.match(r'^[ \t]*class \w+$', cmd)):
                    self.insert('):')
                else:
                    self.insert(self.closing[i])
            # FIXES #8392 (automatically removes the redundant char
            # when autoclosing brackets option is enabled)
            elif t in [')', ']', '}'] and self.autoCloseBracket:
                try:
                    if cmd[index - 1] in self.opening and t == cmd[index]:
                        self.setCursorPosition(line, index + 1)
                        self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
                except IndexError:
                    pass
            elif t == ' ' and self.autoImport:
                ptrn = r'^[ \t]*from [\w.]+$'
                if re.match(ptrn, cmd):
                    self.insert(' import')
                    self.setCursorPosition(line, index + 7)
            QsciScintilla.keyPressEvent(self, e)

        self.displayPrompt(self.continuationLine)

    def populateContextMenu(self, menu):
        pyQGISHelpAction = menu.addAction(
            QgsApplication.getThemeIcon("console/iconHelpConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Search Selected in PyQGIS docs"),
            self.searchSelectedTextInPyQGISDocs
        )
        pyQGISHelpAction.setEnabled(self.hasSelectedText())

    def mousePressEvent(self, e):
        """
        Re-implemented to handle the mouse press event.
        e: the mouse press event (QMouseEvent)
        """
        self.setFocus()
        if e.button() == Qt.MidButton:
            stringSel = QApplication.clipboard().text(QClipboard.Selection)
            if not self.isCursorOnLastLine():
                self.moveCursorToEnd()
            self.insertFromDropPaste(stringSel)
            e.accept()
        else:
            QsciScintilla.mousePressEvent(self, e)

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
            e.setDropAction(Qt.CopyAction)
            e.accept()
        else:
            QsciScintilla.dropEvent(self, e)

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

    def runCommand(self, cmd):
        self.writeCMD(cmd)
        import webbrowser
        self.updateHistory([cmd])
        version = 'master' if 'master' in Qgis.QGIS_VERSION.lower() else \
            re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]
        if cmd in ('_pyqgis', '_api', '_cookbook'):
            if cmd == '_pyqgis':
                webbrowser.open("https://qgis.org/pyqgis/{}".format(version))
            elif cmd == '_api':
                webbrowser.open(
                    "https://qgis.org/api/{}".format('' if version == 'master' else version))
            elif cmd == '_cookbook':
                webbrowser.open(
                    "https://docs.qgis.org/{}/en/docs/pyqgis_developer_cookbook/".format(
                        'testing' if version == 'master' else version))
        else:
            self.buffer.append(cmd)
            src = "\n".join(self.buffer)
            more = self.runsource(src)
            self.continuationLine = more
            if not more:
                self.buffer = []

        # prevents to commands with more lines to break the console
        # in the case they have a eol different from '\n'
        self.setText('')
        self.moveCursorToEnd()
        self.displayPrompt(self.continuationLine)

    def write(self, txt):
        if sys.stderr:
            sys.stderr.write(txt)

    def writeCMD(self, txt):
        if sys.stdout:
            sys.stdout.fire_keyboard_interrupt = False
        if len(txt) > 0:
            prompt = "... " if self.continuationLine else ">>> "
            sys.stdout.write(prompt + txt + '\n')

    def runsource(self, source, filename='<input>', symbol='single'):
        if sys.stdout:
            sys.stdout.fire_keyboard_interrupt = False
        hook = sys.excepthook
        try:
            def excepthook(etype, value, tb):
                self.write("".join(traceback.format_exception(etype, value, tb)))

            sys.excepthook = excepthook

            return super(ShellScintilla, self).runsource(source, filename, symbol)
        finally:
            sys.excepthook = hook
