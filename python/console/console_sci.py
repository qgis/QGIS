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
    QgsCodeEditor,
    QgsCodeInterpreter
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


class PythonInterpreter(QgsCodeInterpreter, code.InteractiveInterpreter):

    def __init__(self):
        super(QgsCodeInterpreter, self).__init__()
        code.InteractiveInterpreter.__init__(self, locals=None)

        self.buffer = []

        for statement in _init_statements:
            try:
                self.runsource(statement)
            except ModuleNotFoundError:
                pass

    def execCommandImpl(self, cmd):
        res = self.currentState()

        self.writeCMD(cmd)
        import webbrowser
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
            res = self.runsource(src)
            if res == 0:
                self.buffer = []

        return res

    def writeCMD(self, txt):
        if sys.stdout:
            sys.stdout.fire_keyboard_interrupt = False
        if len(txt) > 0:
            prompt = "... " if self.currentState() == 1 else ">>> "
            sys.stdout.write(prompt + txt + '\n')

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

    def promptForState(self, state):
        return "..." if state == 1 else ">>>"


class ShellScintilla(QgsCodeEditorPython):

    def __init__(self, parent=None):
        super().__init__(parent, [], QgsCodeEditor.Mode.CommandInput)

        self.parent = parent
        self._interpreter = PythonInterpreter()
        self.setInterpreter(self._interpreter)

        self.opening = ['(', '{', '[', "'", '"']
        self.closing = [')', '}', ']', "'", '"']

        self.settings = QgsSettings()

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
        self.newShortcutCSS = QShortcut(QKeySequence(Qt.CTRL + Qt.SHIFT + Qt.Key_Space), self)
        self.newShortcutCAS = QShortcut(QKeySequence(Qt.CTRL + Qt.ALT + Qt.Key_Space), self)
        self.newShortcutCSS.setContext(Qt.WidgetShortcut)
        self.newShortcutCAS.setContext(Qt.WidgetShortcut)
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
        # update the live history
        self.updateSoftHistory()

        # keyboard interrupt
        if e.modifiers() & (
                Qt.ControlModifier | Qt.MetaModifier) and e.key() == Qt.Key_C and not self.hasSelectedText():
            sys.stdout.fire_keyboard_interrupt = True
            return

        QgsCodeEditorPython.keyPressEvent(self, e)
        self.updatePrompt()

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
            e.setDropAction(Qt.CopyAction)
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
