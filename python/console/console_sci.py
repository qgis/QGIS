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

from qgis.PyQt.QtCore import Qt, QByteArray, QCoreApplication, QFile, QSize
from qgis.PyQt.QtWidgets import QDialog, QMenu, QShortcut, QApplication
from qgis.PyQt.QtGui import QKeySequence, QFontMetrics, QStandardItemModel, QStandardItem, QClipboard
from qgis.PyQt.Qsci import QsciScintilla
from qgis.gui import (
    QgsCodeEditorPython,
    QgsCodeEditorColorScheme
)

import sys
import os
import code
import codecs
import re
import traceback

from qgis.core import QgsApplication, QgsSettings, Qgis
from qgis.gui import QgsCodeEditor

from .ui_console_history_dlg import Ui_HistoryDialogPythonConsole

_init_commands = ["import sys", "import os", "from pathlib import Path", "import re", "import math", "from qgis.core import *",
                  "from qgis.gui import *", "from qgis.analysis import *", "from qgis._3d import *",
                  "import processing", "import qgis.utils",
                  "from qgis.utils import iface", "from qgis.PyQt.QtCore import *", "from qgis.PyQt.QtGui import *",
                  "from qgis.PyQt.QtWidgets import *",
                  "from qgis.PyQt.QtNetwork import *", "from qgis.PyQt.QtXml import *"]
_historyFile = os.path.join(QgsApplication.qgisSettingsDirPath(), "console_history.txt")


class ShellScintilla(QgsCodeEditorPython, code.InteractiveInterpreter):

    def __init__(self, parent=None):
        super(QgsCodeEditorPython, self).__init__(parent)
        code.InteractiveInterpreter.__init__(self, locals=None)

        self.parent = parent

        self.opening = ['(', '{', '[', "'", '"']
        self.closing = [')', '}', ']', "'", '"']

        self.settings = QgsSettings()

        self.new_input_line = True

        self.buffer = []
        self.continuationLine = False

        self.displayPrompt(self.continuationLine)

        for line in _init_commands:
            try:
                self.runsource(line)
            except ModuleNotFoundError:
                pass

        self.history = []
        self.softHistory = ['']
        self.softHistoryIndex = 0
        # Read history command file
        self.readHistoryFile()

        self.historyDlg = HistoryDialog(self)

        self.refreshSettingsShell()

        # Don't want to see the horizontal scrollbar at all
        # Use raw message to Scintilla here (all messages are documented
        # here: http://www.scintilla.org/ScintillaDoc.html)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        # not too small
        # self.setMinimumSize(500, 300)

        self.setWrapMode(QsciScintilla.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_EMPTYUNDOBUFFER)

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

    def showHistory(self):
        if not self.historyDlg.isVisible():
            self.historyDlg.show()
        self.historyDlg._reloadHistory()
        self.historyDlg.activateWindow()

    def commandConsole(self, commands):
        if not self.is_cursor_on_last_line():
            self.move_cursor_to_end()
        for cmd in commands:
            self.setText(cmd)
            self.entered()
        self.move_cursor_to_end()
        self.setFocus()

    def getText(self):
        """ Get the text as a unicode string. """
        value = self.getBytes().decode('utf-8')
        # print (value) printing can give an error because the console font
        # may not have all unicode characters
        return value

    def getBytes(self):
        """ Get the text as bytes (utf-8 encoded). This is how
        the data is stored internally. """
        len = self.SendScintilla(self.SCI_GETLENGTH) + 1
        bb = QByteArray(len, '0')
        self.SendScintilla(self.SCI_GETTEXT, len, bb)
        return bytes(bb)[:-1]

    def getTextLength(self):
        return self.SendScintilla(QsciScintilla.SCI_GETLENGTH)

    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.lines() - 1
        return line, len(self.text(line))

    def is_cursor_at_start(self):
        """Return True if cursor is at the end of text"""
        cline, cindex = self.getCursorPosition()
        return cline == 0 and cindex == 0

    def is_cursor_at_end(self):
        """Return True if cursor is at the end of text"""
        cline, cindex = self.getCursorPosition()
        return (cline, cindex) == self.get_end_pos()

    def move_cursor_to_start(self):
        """Move cursor to start of text"""
        self.setCursorPosition(0, 0)
        self.ensureCursorVisible()
        self.ensureLineVisible(0)
        self.displayPrompt(self.continuationLine)

    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.setCursorPosition(line, index)
        self.ensureCursorVisible()
        self.ensureLineVisible(line)
        self.displayPrompt(self.continuationLine)

    def is_cursor_on_last_line(self):
        """Return True if cursor is on the last line"""
        cline, _ = self.getCursorPosition()
        return cline == self.lines() - 1

    def new_prompt(self, prompt):
        """
        Print a new prompt and save its (line, index) position
        """
        self.write(prompt, prompt=True)
        # now we update our cursor giving end of prompt
        line, index = self.getCursorPosition()
        self.ensureCursorVisible()
        self.ensureLineVisible(line)

    def displayPrompt(self, more=False):
        self.SendScintilla(QsciScintilla.SCI_MARGINSETTEXT, 0, str.encode("..." if more else ">>>"))

    def syncSoftHistory(self):
        self.softHistory = self.history[:]
        self.softHistory.append('')
        self.softHistoryIndex = len(self.softHistory) - 1

    def updateSoftHistory(self):
        self.softHistory[self.softHistoryIndex] = self.text()

    def updateHistory(self, command, skipSoftHistory=False):
        if isinstance(command, list):
            for line in command:
                self.history.append(line)
        elif not command == "":
            if len(self.history) <= 0 or \
                    command != self.history[-1]:
                self.history.append(command)
        if not skipSoftHistory:
            self.syncSoftHistory()

    def writeHistoryFile(self, fromCloseConsole=False):
        ok = False
        try:
            wH = codecs.open(_historyFile, 'w', encoding='utf-8')
            for s in self.history:
                wH.write(s + '\n')
            ok = True
        except:
            raise
        wH.close()
        if ok and not fromCloseConsole:
            msgText = QCoreApplication.translate('PythonConsole',
                                                 'History saved successfully.')
            self.parent.callWidgetMessageBar(msgText)

    def readHistoryFile(self):
        fileExist = QFile.exists(_historyFile)
        if fileExist:
            with codecs.open(_historyFile, 'r', encoding='utf-8') as rH:
                for line in rH:
                    if line != "\n":
                        l = line.rstrip('\n')
                        self.updateHistory(l, True)
            self.syncSoftHistory()
        else:
            return

    def clearHistory(self, clearSession=False):
        if clearSession:
            self.history = []
            self.syncSoftHistory()
            msgText = QCoreApplication.translate('PythonConsole',
                                                 'Session and file history cleared successfully.')
            self.parent.callWidgetMessageBar(msgText)
            return
        ok = False
        try:
            cH = codecs.open(_historyFile, 'w', encoding='utf-8')
            ok = True
        except:
            raise
        cH.close()
        if ok:
            msgText = QCoreApplication.translate('PythonConsole',
                                                 'History cleared successfully.')
            self.parent.callWidgetMessageBar(msgText)

    def clearHistorySession(self):
        self.clearHistory(True)

    def showPrevious(self):
        if self.softHistoryIndex < len(self.softHistory) - 1 and self.softHistory:
            self.softHistoryIndex += 1
            self.setText(self.softHistory[self.softHistoryIndex])
            self.move_cursor_to_end()
            # self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def showNext(self):
        if self.softHistoryIndex > 0 and self.softHistory:
            self.softHistoryIndex -= 1
            self.setText(self.softHistory[self.softHistoryIndex])
            self.move_cursor_to_end()
            # self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

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
            self.move_cursor_to_end()

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
            self.showPrevious()

        elif e.key() == Qt.Key_Up and not self.isListActive():
            self.showNext()

        # TODO: press event for auto-completion file directory
        else:
            t = e.text()
            self.autoCloseBracket = self.settings.value("pythonConsole/autoCloseBracket", False, type=bool)
            self.autoImport = self.settings.value("pythonConsole/autoInsertionImport", True, type=bool)
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

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        subMenu = QMenu(menu)
        titleHistoryMenu = QCoreApplication.translate("PythonConsole", "Command History")
        subMenu.setTitle(titleHistoryMenu)
        subMenu.addAction(
            QCoreApplication.translate("PythonConsole", "Show"),
            self.showHistory, 'Ctrl+Shift+SPACE')
        subMenu.addAction(
            QCoreApplication.translate("PythonConsole", "Clear File"),
            self.clearHistory)
        subMenu.addAction(
            QCoreApplication.translate("PythonConsole", "Clear Session"),
            self.clearHistorySession)
        menu.addMenu(subMenu)
        menu.addSeparator()
        copyAction = menu.addAction(
            QgsApplication.getThemeIcon("mActionEditCopy.svg"),
            QCoreApplication.translate("PythonConsole", "Copy"),
            self.copy, QKeySequence.Copy)
        pasteAction = menu.addAction(
            QgsApplication.getThemeIcon("mActionEditPaste.svg"),
            QCoreApplication.translate("PythonConsole", "Paste"),
            self.paste, QKeySequence.Paste)
        pyQGISHelpAction = menu.addAction(QgsApplication.getThemeIcon("console/iconHelpConsole.svg"),
                                          QCoreApplication.translate("PythonConsole", "Search Selected in PyQGIS docs"),
                                          self.searchSelectedTextInPyQGISDocs)
        copyAction.setEnabled(False)
        pasteAction.setEnabled(False)
        pyQGISHelpAction.setEnabled(False)
        if self.hasSelectedText():
            copyAction.setEnabled(True)
            pyQGISHelpAction.setEnabled(True)
        if QApplication.clipboard().text():
            pasteAction.setEnabled(True)
        menu.exec_(self.mapToGlobal(e.pos()))

    def mousePressEvent(self, e):
        """
        Re-implemented to handle the mouse press event.
        e: the mouse press event (QMouseEvent)
        """
        self.setFocus()
        if e.button() == Qt.MidButton:
            stringSel = QApplication.clipboard().text(QClipboard.Selection)
            if not self.is_cursor_on_last_line():
                self.move_cursor_to_end()
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
        if self.is_cursor_on_last_line():
            if self.hasSelectedText():
                self.removeSelectedText()
        else:
            self.move_cursor_to_end()
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
                self.move_cursor_to_end()
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
            self.move_cursor_to_end()
            self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            self.runCommand(self.text())
        self.append(listOpenFile[-1])
        self.move_cursor_to_end()
        self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def entered(self):
        self.move_cursor_to_end()
        self.runCommand(self.text())
        self.setFocus()
        self.move_cursor_to_end()

    def runCommand(self, cmd):
        self.writeCMD(cmd)
        import webbrowser
        self.updateHistory(cmd)
        version = 'master' if 'master' in Qgis.QGIS_VERSION.lower() else re.findall(r'^\d.[0-9]*', Qgis.QGIS_VERSION)[0]
        if cmd in ('_pyqgis', '_api', '_cookbook'):
            if cmd == '_pyqgis':
                webbrowser.open("https://qgis.org/pyqgis/{}".format(version))
            elif cmd == '_api':
                webbrowser.open("https://qgis.org/api/{}".format('' if version == 'master' else version))
            elif cmd == '_cookbook':
                webbrowser.open("https://docs.qgis.org/{}/en/docs/pyqgis_developer_cookbook/".format(
                    'testing' if version == 'master' else version))
        else:
            self.buffer.append(cmd)
            src = "\n".join(self.buffer)
            more = self.runsource(src)
            self.continuationLine = True
            if not more:
                self.continuationLine = False
                self.buffer = []

        # prevents to commands with more lines to break the console
        # in the case they have a eol different from '\n'
        self.setText('')
        self.move_cursor_to_end()
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


class HistoryDialog(QDialog, Ui_HistoryDialogPythonConsole):

    def __init__(self, parent):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        self.parent = parent
        self.setWindowTitle(QCoreApplication.translate("PythonConsole",
                                                       "Python Console - Command History"))
        self.listView.setToolTip(QCoreApplication.translate("PythonConsole",
                                                            "Double-click on item to execute"))

        self.listView.setFont(QgsCodeEditorPython.getMonospaceFont())

        self.model = QStandardItemModel(self.listView)

        self._reloadHistory()

        self.deleteScut = QShortcut(QKeySequence(Qt.Key_Delete), self)
        self.deleteScut.activated.connect(self._deleteItem)
        self.listView.doubleClicked.connect(self._runHistory)
        self.reloadHistory.clicked.connect(self._reloadHistory)
        self.saveHistory.clicked.connect(self._saveHistory)
        self.runHistoryButton.clicked.connect(self._executeSelectedHistory)

    def _executeSelectedHistory(self):
        items = self.listView.selectionModel().selectedIndexes()
        items.sort()
        for item in items:
            self.parent.runCommand(item.data(Qt.DisplayRole))

    def _runHistory(self, item):
        cmd = item.data(Qt.DisplayRole)
        self.parent.runCommand(cmd)

    def _saveHistory(self):
        self.parent.writeHistoryFile(True)

    def _reloadHistory(self):
        self.model.clear()
        item = None
        for i in self.parent.history:
            item = QStandardItem(i)
            if sys.platform.startswith('win'):
                item.setSizeHint(QSize(18, 18))
            self.model.appendRow(item)

        self.listView.setModel(self.model)
        self.listView.scrollToBottom()
        if item:
            self.listView.setCurrentIndex(self.model.indexFromItem(item))

    def _deleteItem(self):
        itemsSelected = self.listView.selectionModel().selectedIndexes()
        if itemsSelected:
            item = itemsSelected[0].row()
            # Remove item from the command history (just for the current session)
            self.parent.history.pop(item)
            self.parent.softHistory.pop(item)
            if item < self.parent.softHistoryIndex:
                self.parent.softHistoryIndex -= 1
            self.parent.setText(self.parent.softHistory[self.parent.softHistoryIndex])
            self.parent.move_cursor_to_end()
            # Remove row from the command history dialog
            self.model.removeRow(item)
