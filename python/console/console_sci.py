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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.Qsci import (QsciScintilla,
                        QsciScintillaBase,
                        QsciLexerPython,
                        QsciAPIs)

import sys
import os
import code

from qgis.core import QgsApplication

_init_commands = ["from qgis.core import *", "import qgis.utils",
                  "from qgis.utils import iface"]
_historyFile = unicode( QgsApplication.qgisSettingsDirPath() + "console_history.txt" )

class ShellScintilla(QsciScintilla, code.InteractiveInterpreter):
    def __init__(self, parent=None):
        super(ShellScintilla,self).__init__(parent)
        code.InteractiveInterpreter.__init__(self, locals=None)

        self.parent = parent

        self.opening = ['(', '{', '[', "'", '"']
        self.closing = [')', '}', ']', "'", '"']

        self.settings = QSettings()

        # Enable non-ascii chars for editor
        self.setUtf8(True)

        self.new_input_line = True

        self.setMarginWidth(0, 0)
        self.setMarginWidth(1, 0)
        self.setMarginWidth(2, 0)

        self.buffer = []

        self.displayPrompt(False)

        for line in _init_commands:
            self.runsource(line)

        self.history = QStringList()
        self.historyIndex = 0
        # Read history command file
        self.readHistoryFile()

        # Brace matching: enable for a brace immediately before or after
        # the current position
        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)
        self.setMatchedBraceBackgroundColor(QColor("#b7f907"))

        # Current line visible with special background color
        self.setCaretWidth(2)

        self.settingsShell()

        # Don't want to see the horizontal scrollbar at all
        # Use raw message to Scintilla here (all messages are documented
        # here: http://www.scintilla.org/ScintillaDoc.html)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        # not too small
        #self.setMinimumSize(500, 300)
        self.setMinimumHeight(20)

        self.setWrapMode(QsciScintilla.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_EMPTYUNDOBUFFER)

        ## Disable command key
        ctrl, shift = self.SCMOD_CTRL<<16, self.SCMOD_SHIFT<<16
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Z')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('Y')+ ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L')+ ctrl+shift)

        ## New QShortcut = ctrl+space/ctrl+alt+space for Autocomplete
        self.newShortcutCSS = QShortcut(QKeySequence(Qt.CTRL + Qt.SHIFT + Qt.Key_Space), self)
        self.newShortcutCAS = QShortcut(QKeySequence(Qt.CTRL + Qt.ALT + Qt.Key_Space), self)
        self.newShortcutCSS.setContext(Qt.WidgetShortcut)
        self.newShortcutCAS.setContext(Qt.WidgetShortcut)
        self.newShortcutCAS.activated.connect(self.autoCompleteKeyBinding)
        self.newShortcutCSS.activated.connect(self.showHistory)
        self.connect(self, SIGNAL('userListActivated(int, const QString)'),
                     self.completion_list_selected)

    def settingsShell(self):
        # Set Python lexer
        self.setLexers()
        threshold = self.settings.value("pythonConsole/autoCompThreshold", 2).toInt()[0]
        self.setAutoCompletionThreshold(threshold)
        radioButtonSource = self.settings.value("pythonConsole/autoCompleteSource", 'fromAPI').toString()
        autoCompEnabled = self.settings.value("pythonConsole/autoCompleteEnabled", True).toBool()
        self.setAutoCompletionThreshold(threshold)
        if autoCompEnabled:
            if radioButtonSource == 'fromDoc':
                self.setAutoCompletionSource(self.AcsDocument)
            elif radioButtonSource == 'fromAPI':
                self.setAutoCompletionSource(self.AcsAPIs)
            elif radioButtonSource == 'fromDocAPI':
                self.setAutoCompletionSource(self.AcsAll)
        else:
            self.setAutoCompletionSource(self.AcsNone)

    def showHistory(self):
        self.showUserList(1, QStringList(self.history))

    def autoCompleteKeyBinding(self):
        radioButtonSource = self.settings.value("pythonConsole/autoCompleteSource").toString()
        autoCompEnabled = self.settings.value("pythonConsole/autoCompleteEnabled").toBool()
        if autoCompEnabled:
            if radioButtonSource == 'fromDoc':
                self.autoCompleteFromDocument()
            elif radioButtonSource == 'fromAPI':
                self.autoCompleteFromAPIs()
            elif radioButtonSource == 'fromDocAPI':
                self.autoCompleteFromAll()

    def commandConsole(self, command):
        if not self.is_cursor_on_last_line():
            self.move_cursor_to_end()
        line, pos = self.getCursorPosition()
        selCmdLenght = self.text(line).length()
        self.setSelection(line, 4, line, selCmdLenght)
        self.removeSelectedText()
        if command == "sextante":
            # import Sextante class
            self.append('import sextante')
        elif command == "qtCore":
            # import QtCore class
            self.append('from PyQt4.QtCore import *')
        elif command == "qtGui":
            # import QtGui class
            self.append('from PyQt4.QtGui import *')
        self.entered()
        self.move_cursor_to_end()
        self.setFocus()

    def setLexers(self):
        self.lexer = QsciLexerPython()

        loadFont = self.settings.value("pythonConsole/fontfamilytext", "Monospace").toString()
        fontSize = self.settings.value("pythonConsole/fontsize", 10).toInt()[0]

        font = QFont(loadFont)
        font.setFixedPitch(True)
        font.setPointSize(fontSize)
        font.setStyleHint(QFont.TypeWriter)
        font.setStretch(QFont.SemiCondensed)
        font.setLetterSpacing(QFont.PercentageSpacing, 87.0)
        font.setBold(False)

        self.lexer.setDefaultFont(font)
        self.lexer.setColor(Qt.red, 1)
        self.lexer.setColor(Qt.darkGreen, 5)
        self.lexer.setColor(Qt.darkBlue, 15)
        self.lexer.setFont(font, 1)
        self.lexer.setFont(font, 3)
        self.lexer.setFont(font, 4)

        self.api = QsciAPIs(self.lexer)
        chekBoxAPI = self.settings.value("pythonConsole/preloadAPI", True).toBool()
        if chekBoxAPI:
            self.api.loadPrepared( QgsApplication.pkgDataPath() + "/python/qsci_apis/pyqgis_master.pap" )
        else:
            apiPath = self.settings.value("pythonConsole/userAPI").toStringList()
            for i in range(0, len(apiPath)):
                self.api.load(QString(unicode(apiPath[i])))
            self.api.prepare()
            self.lexer.setAPIs(self.api)

        self.setLexer(self.lexer)

    ## TODO: show completion list for file and directory

    def completion_list_selected(self, id, txt):
        if id == 1:
            txt = unicode(txt)
            # get current cursor position
            line, pos = self.getCursorPosition()
            selCmdLength = self.text(line).length()
            # select typed text
            self.setSelection(line, 4, line, selCmdLength)
            self.removeSelectedText()
            self.insert(txt)

    def getText(self):
        """ Get the text as a unicode string. """
        value = self.getBytes().decode('utf-8')
        # print (value) printing can give an error because the console font
        # may not have all unicode characters
        return value

    def getBytes(self):
        """ Get the text as bytes (utf-8 encoded). This is how
        the data is stored internally. """
        len = self.SendScintilla(self.SCI_GETLENGTH)+1
        bb = QByteArray(len,'0')
        N = self.SendScintilla(self.SCI_GETTEXT, len, bb)
        return bytes(bb)[:-1]

    def getTextLength(self):
        return self.SendScintilla(QsciScintilla.SCI_GETLENGTH)

    def get_end_pos(self):
        """Return (line, index) position of the last character"""
        line = self.lines() - 1
        return (line, self.text(line).length())

    def is_cursor_at_end(self):
        """Return True if cursor is at the end of text"""
        cline, cindex = self.getCursorPosition()
        return (cline, cindex) == self.get_end_pos()

    def move_cursor_to_end(self):
        """Move cursor to end of text"""
        line, index = self.get_end_pos()
        self.setCursorPosition(line, index)
        self.ensureCursorVisible()
        self.ensureLineVisible(line)

    #def on_new_line(self):
        #"""On new input line"""
        #self.move_cursor_to_end()
        #self.new_input_line = False

    def is_cursor_on_last_line(self):
        """Return True if cursor is on the last line"""
        cline, _ = self.getCursorPosition()
        return cline == self.lines() - 1

    def is_cursor_on_edition_zone(self):
        """ Return True if the cursor is in the edition zone """
        cline, cindex = self.getCursorPosition()
        return cline == self.lines() - 1 and cindex >= 4

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
        self.append("... ") if more else self.append(">>> ")
        self.move_cursor_to_end()

    def updateHistory(self, command):
        if isinstance(command, QStringList):
            for line in command:
                self.history.append(line)
        elif not command == "":
            if len(self.history) <= 0 or \
            not command == self.history[-1]:
                self.history.append(command)
        self.historyIndex = len(self.history)

    def writeHistoryFile(self):
        wH = open(_historyFile, 'w')
        for s in self.history:
            wH.write(s + '\n')
        wH.close()

    def readHistoryFile(self):
        fileExist = QFile.exists(_historyFile)
        if fileExist:
            rH = open(_historyFile, 'r')
            for line in rH:
                if line != "\n":
                    l = line.rstrip('\n')
                    self.updateHistory(l)
        else:
            return

    def clearHistoryFile(self):
        cH = open(_historyFile, 'w')
        cH.close()

    def showPrevious(self):
        if self.historyIndex < len(self.history) and not self.history.isEmpty():
            line, pos = self.getCursorPosition()
            selCmdLenght = self.text(line).length()
            self.setSelection(line, 4, line, selCmdLenght)
            self.removeSelectedText()
            self.historyIndex += 1
            if self.historyIndex == len(self.history):
                self.insert("")
                pass
            else:
                self.insert(self.history[self.historyIndex])
            self.move_cursor_to_end()
            #self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def showNext(self):
        if  self.historyIndex > 0 and not self.history.isEmpty():
            line, pos = self.getCursorPosition()
            selCmdLenght = self.text(line).length()
            self.setSelection(line, 4, line, selCmdLenght)
            self.removeSelectedText()
            self.historyIndex -= 1
            if self.historyIndex == len(self.history):
                self.insert("")
            else:
                self.insert(self.history[self.historyIndex])
            self.move_cursor_to_end()
            #self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def keyPressEvent(self, e):
        startLine, _, endLine, _ = self.getSelection()

        # handle invalid cursor position and multiline selections
        if not self.is_cursor_on_edition_zone() or startLine < endLine:
            # allow to copy and select
            if e.modifiers() & (Qt.ControlModifier | Qt.MetaModifier):
                if e.key() in (Qt.Key_C, Qt.Key_A):
                    QsciScintilla.keyPressEvent(self, e)
                return
            # allow selection
            if e.modifiers() & Qt.ShiftModifier:
                if e.key() in (Qt.Key_Left, Qt.Key_Right, Qt.Key_Home, Qt.Key_End):
                    QsciScintilla.keyPressEvent(self, e)
                return
            # all other keystrokes get sent to the input line
            self.move_cursor_to_end()

        line, index = self.getCursorPosition()
        cmd = self.text(line)

        if e.key() in (Qt.Key_Return, Qt.Key_Enter) and not self.isListActive():
            self.entered()

        elif e.key() in (Qt.Key_Left, Qt.Key_Home):
            QsciScintilla.keyPressEvent(self, e)
            # check whether the cursor is moved out of the edition zone
            newline, newindex = self.getCursorPosition()
            if newline < line or newindex < 4:
                # fix selection and the cursor position
                if self.hasSelectedText():
                    self.setSelection(line, self.getSelection()[3], line, 4)
                else:
                    self.setCursorPosition(line, 4)

        elif e.key() in (Qt.Key_Backspace, Qt.Key_Delete):
            QsciScintilla.keyPressEvent(self, e)
            # check whether the cursor is moved out of the edition zone
            _, newindex = self.getCursorPosition()
            if newindex < 4:
                # restore the prompt chars (if removed) and
                # fix the cursor position
                self.insert( cmd[:3-newindex] + " " )
                self.setCursorPosition(line, 4)
            self.recolor()

        elif (e.modifiers() & (Qt.ControlModifier | Qt.MetaModifier) and \
            e.key() == Qt.Key_V) or \
            (e.modifiers() & Qt.ShiftModifier and e.key() == Qt.Key_Insert):
            self.paste()
            e.accept()

        elif e.key() == Qt.Key_Down and not self.isListActive():
            self.showPrevious()
        elif e.key() == Qt.Key_Up and not self.isListActive():
            self.showNext()
        ## TODO: press event for auto-completion file directory
        else:
            if self.settings.value("pythonConsole/autoCloseBracket", True).toBool():
                t = unicode(e.text())
                ## Close bracket automatically
                if t in self.opening:
                    i = self.opening.index(t)
                    self.insert(self.closing[i])
            QsciScintilla.keyPressEvent(self, e)

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        copyAction = menu.addAction("Copy", self.copy, QKeySequence.Copy)
        pasteAction = menu.addAction("Paste", self.paste, QKeySequence.Paste)
        copyAction.setEnabled(False)
        pasteAction.setEnabled(False)
        if self.hasSelectedText():
            copyAction.setEnabled(True)
        if QApplication.clipboard().text():
            pasteAction.setEnabled(True)
        action = menu.exec_(self.mapToGlobal(e.pos()))

    def mousePressEvent(self, e):
        """
        Re-implemented to handle the mouse press event.
        e: the mouse press event (QMouseEvent)
        """
        self.setFocus()
        if e.button() == Qt.MidButton:
            stringSel = unicode(QApplication.clipboard().text(QClipboard.Selection))
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
        stringPaste = unicode(QApplication.clipboard().text())
        if self.is_cursor_on_last_line():
            if self.hasSelectedText():
                self.removeSelectedText()
        else:
            self.move_cursor_to_end()
        self.insertFromDropPaste(stringPaste)

    ## Drag and drop
    def dropEvent(self, e):
        if e.mimeData().hasText():
            stringDrag = e.mimeData().text()
            self.insertFromDropPaste(stringDrag)
            self.setFocus()
            e.setDropAction(Qt.CopyAction)
            e.accept()
        else:
            QsciScintillaCompat.dropEvent(self, e)

    def insertFromDropPaste(self, textDP):
        pasteList = str(textDP).splitlines()
        for line in pasteList[:-1]:
            line.replace(">>> ", "").replace("... ", "")
            self.insert(unicode(line))
            self.move_cursor_to_end()
            self.runCommand(unicode(self.currentCommand()))
        if pasteList[-1] != "":
            line = pasteList[-1]
            line.replace(">>> ", "").replace("... ", "")
            self.insert(unicode(line))
            self.move_cursor_to_end()

    def insertTextFromFile(self, listOpenFile):
        for line in listOpenFile[:-1]:
            self.append(line)
            self.move_cursor_to_end()
            self.SendScintilla(QsciScintilla.SCI_DELETEBACK)
            self.runCommand(unicode(self.currentCommand()))
        self.append(unicode(listOpenFile[-1]))
        self.move_cursor_to_end()
        self.SendScintilla(QsciScintilla.SCI_DELETEBACK)

    def entered(self):
        self.move_cursor_to_end()
        self.runCommand( unicode(self.currentCommand()) )
        self.setFocus()
        self.move_cursor_to_end()
        #self.SendScintilla(QsciScintilla.SCI_EMPTYUNDOBUFFER)

    def currentCommand(self):
        linenr, index = self.getCursorPosition()
        #for i in range(0, linenr):
        txtLength = self.text(linenr).length()
        string = self.text()
        cmdLine = string.right(txtLength - 4)
        cmd = unicode(cmdLine)
        return cmd

    def runCommand(self, cmd):
        self.writeCMD(cmd)
        import webbrowser
        self.updateHistory(cmd)
        if cmd in ('_save', '_clear', '_clearAll', '_pyqgis', '_api'):
            if cmd == '_save':
                self.writeHistoryFile()
                msgText = QCoreApplication.translate('PythonConsole', 'History saved successfully.')
            elif cmd == '_clear':
                self.clearHistoryFile()
                msgText = QCoreApplication.translate('PythonConsole', 'History cleared successfully.')
            elif cmd == '_clearAll':
                self.history = QStringList()
                self.clearHistoryFile()
                msgText = QCoreApplication.translate('PythonConsole', 'Session and file history cleared successfully.')
            elif cmd == '_pyqgis':
                webbrowser.open( "http://www.qgis.org/pyqgis-cookbook/" )
            elif cmd == '_api':
                webbrowser.open( "http://www.qgis.org/api/" )
            if msgText:
                self.parent.callWidgetMessageBar(msgText)

            more = False
        else:
            self.buffer.append(cmd)
            src = u"\n".join(self.buffer)
            more = self.runsource(src, "<input>")
            if not more:
                self.buffer = []
        ## prevents to commands with more lines to break the console
        ## in the case they have a eol different from '\n'
        self.setText('')
        self.move_cursor_to_end()
        self.displayPrompt(more)

    def write(self, txt):
        sys.stderr.write(txt)

    def writeCMD(self, txt):
        if len(txt) > 0:
            getCmdString = self.text()
            prompt = getCmdString[0:4]
            sys.stdout.write(prompt+txt+'\n')
