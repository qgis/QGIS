# -*- coding: utf-8 -*-

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Some portions of code were taken from managerR plugin.

"""
Implementation of interactive Python console widget for QGIS.

Has +- the same behaviour as command-line interactive console:
- runs commands one by one
- supports expressions that span through more lines
- has command history, accessible using up/down keys
- supports pasting of commands

TODO:
- configuration - init commands, font, ...
- python code highlighting

"""

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.utils import iface
import sys
import traceback
import code


_init_commands = ["from qgis.core import *", "import qgis.utils"]

_console = None

def show_console():
  """ called from QGIS to open the console """
  global _console
  if _console is None:
    _console = PythonConsole(iface.mainWindow())
    _console.show() # force show even if it was restored as hidden
  else:
    _console.setVisible(not _console.isVisible())
  # set focus to the edit box so the user can start typing
  if _console.isVisible():
    _console.activateWindow()
    _console.edit.setFocus()

  
_old_stdout = sys.stdout
_console_output = None


def clearConsole():
  global _console
  if _console is None:
    return
  _console.edit.clearConsole()


# hook for python console so all output will be redirected
# and then shown in console
def console_displayhook(obj):
  global _console_output
  _console_output = obj

class QgisOutputCatcher:
  def __init__(self):
    self.data = ''
  def write(self, stuff):
    self.data += stuff
  def get_and_clean_data(self):
    tmp = self.data
    self.data = ''
    return tmp
  def flush(self):
    pass

sys.stdout = QgisOutputCatcher()

class PythonConsole(QDockWidget):
  def __init__(self, parent=None):
    QDockWidget.__init__(self, parent)
    self.setObjectName("Python Console")
    self.setAllowedAreas(Qt.BottomDockWidgetArea)
    self.widget = QWidget()
    self.l = QVBoxLayout(self.widget)
    self.edit = PythonEdit()
    self.l.addWidget(self.edit)
    self.setWidget(self.widget)
    self.setWindowTitle(QCoreApplication.translate("PythonConsole", "Python Console"))
    # try to restore position from stored main window state
    if not iface.mainWindow().restoreDockWidget(self):
      iface.mainWindow().addDockWidget(Qt.BottomDockWidgetArea, self)
    

  def sizeHint(self):
    return QSize(500,300)

  def closeEvent(self, event):
    QWidget.closeEvent(self, event)


class ConsoleHighlighter(QSyntaxHighlighter):
  EDIT_LINE, ERROR, OUTPUT, INIT = range(4)
  def __init__(self, doc):
    QSyntaxHighlighter.__init__(self,doc)
    formats = { self.OUTPUT    : Qt.black,
		self.ERROR     : Qt.red,
		self.EDIT_LINE : Qt.darkGreen,
		self.INIT      : Qt.gray }
    self.f = {}
    for tag, color in formats.iteritems():
      self.f[tag] = QTextCharFormat()
      self.f[tag].setForeground(color)

  def highlightBlock(self, txt):
    size = txt.length()
    state = self.currentBlockState()
    if state == self.OUTPUT or state == self.ERROR or state == self.INIT:
      self.setFormat(0,size, self.f[state])
    # highlight prompt only
    if state == self.EDIT_LINE:
      self.setFormat(0,3, self.f[self.EDIT_LINE])


class PythonEdit(QTextEdit, code.InteractiveInterpreter):

  def __init__(self,parent=None):
    QTextEdit.__init__(self, parent)
    code.InteractiveInterpreter.__init__(self, locals=None)

    self.setTextInteractionFlags(Qt.TextEditorInteraction)
    self.setAcceptDrops(False)
    self.setMinimumSize(30, 30)
    self.setUndoRedoEnabled(False)
    self.setAcceptRichText(False)
    monofont = QFont("Monospace")
    monofont.setStyleHint(QFont.TypeWriter)
    self.setFont(monofont)

    self.buffer = []
    
    self.insertInitText()

    for line in _init_commands:
      self.runsource(line)

    self.displayPrompt(False)

    self.history = QStringList()
    self.historyIndex = 0

    self.high = ConsoleHighlighter(self)
    
  def insertInitText(self):
    self.insertTaggedText(QCoreApplication.translate("PythonConsole", "To access Quantum GIS environment from this console\n"
                          "use qgis.utils.iface object (instance of QgisInterface class).\n\n"),
                          ConsoleHighlighter.INIT)


  def clearConsole(self):
    self.clear()
    self.insertInitText()

  def displayPrompt(self, more=False):
    self.currentPrompt = "... " if more else ">>> "
    self.currentPromptLength = len(self.currentPrompt)
    self.insertTaggedLine(self.currentPrompt, ConsoleHighlighter.EDIT_LINE)
    self.moveCursor(QTextCursor.End, QTextCursor.MoveAnchor)

  def isCursorInEditionZone(self):
    cursor = self.textCursor()
    pos = cursor.position()
    block = self.document().lastBlock()
    last = block.position() + self.currentPromptLength
    return pos >= last

  def currentCommand(self):
    block = self.cursor.block()
    text = block.text()
    return text.right(text.length()-self.currentPromptLength)

  def showPrevious(self):
        if self.historyIndex < len(self.history) and not self.history.isEmpty():
            self.cursor.movePosition(QTextCursor.EndOfBlock, QTextCursor.MoveAnchor)
            self.cursor.movePosition(QTextCursor.StartOfBlock, QTextCursor.KeepAnchor)
            self.cursor.removeSelectedText()
            self.cursor.insertText(self.currentPrompt)
            self.historyIndex += 1
            if self.historyIndex == len(self.history):
                self.insertPlainText("")
            else:
                self.insertPlainText(self.history[self.historyIndex])

  def showNext(self):
        if  self.historyIndex > 0 and not self.history.isEmpty():
            self.cursor.movePosition(QTextCursor.EndOfBlock, QTextCursor.MoveAnchor)
            self.cursor.movePosition(QTextCursor.StartOfBlock, QTextCursor.KeepAnchor)
            self.cursor.removeSelectedText()
            self.cursor.insertText(self.currentPrompt)
            self.historyIndex -= 1
            if self.historyIndex == len(self.history):
                self.insertPlainText("")
            else:
                self.insertPlainText(self.history[self.historyIndex])

  def updateHistory(self, command):
        if isinstance(command, QStringList):
            for line in command:
                self.history.append(line)
        elif not command == "":
            if len(self.history) <= 0 or \
            not command == self.history[-1]:
                self.history.append(command)
        self.historyIndex = len(self.history)

  def keyPressEvent(self, e):
        self.cursor = self.textCursor()
        # if the cursor isn't in the edition zone, don't do anything except Ctrl+C
        if not self.isCursorInEditionZone():
            if e.modifiers() & Qt.ControlModifier or e.modifiers() & Qt.MetaModifier:
                if e.key() == Qt.Key_C or e.key() == Qt.Key_A:
                    QTextEdit.keyPressEvent(self, e)
            else:
                # all other keystrokes get sent to the input line
                self.cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
        else:
            # if Return is pressed, then perform the commands
            if e.key() == Qt.Key_Return:
                self.entered()
            # if Up or Down is pressed
            elif e.key() == Qt.Key_Down:
                self.showPrevious()
            elif e.key() == Qt.Key_Up:
                self.showNext()
            # if backspace is pressed, delete until we get to the prompt
            elif e.key() == Qt.Key_Backspace:
                if not self.cursor.hasSelection() and self.cursor.columnNumber() == self.currentPromptLength:
                    return
                QTextEdit.keyPressEvent(self, e)
            # if the left key is pressed, move left until we get to the prompt
            elif e.key() == Qt.Key_Left and self.cursor.position() > self.document().lastBlock().position() + self.currentPromptLength:
                anchor = QTextCursor.KeepAnchor if e.modifiers() & Qt.ShiftModifier else QTextCursor.MoveAnchor
                move = QTextCursor.WordLeft if e.modifiers() & Qt.ControlModifier or e.modifiers() & Qt.MetaModifier else QTextCursor.Left
                self.cursor.movePosition(move, anchor)
            # use normal operation for right key
            elif e.key() == Qt.Key_Right:
                anchor = QTextCursor.KeepAnchor if e.modifiers() & Qt.ShiftModifier else QTextCursor.MoveAnchor
                move = QTextCursor.WordRight if e.modifiers() & Qt.ControlModifier or e.modifiers() & Qt.MetaModifier else QTextCursor.Right
                self.cursor.movePosition(move, anchor)
            # if home is pressed, move cursor to right of prompt
            elif e.key() == Qt.Key_Home:
                anchor = QTextCursor.KeepAnchor if e.modifiers() & Qt.ShiftModifier else QTextCursor.MoveAnchor
                self.cursor.movePosition(QTextCursor.StartOfBlock, anchor, 1)
                self.cursor.movePosition(QTextCursor.Right, anchor, self.currentPromptLength)
            # use normal operation for end key
            elif e.key() == Qt.Key_End:
                anchor = QTextCursor.KeepAnchor if e.modifiers() & Qt.ShiftModifier else QTextCursor.MoveAnchor
                self.cursor.movePosition(QTextCursor.EndOfBlock, anchor, 1)
            # use normal operation for all remaining keys
            else:
                QTextEdit.keyPressEvent(self, e)
        self.setTextCursor(self.cursor)
        self.ensureCursorVisible()

  def insertFromMimeData(self, source):
        self.cursor = self.textCursor()
        self.cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor, 1)
        self.setTextCursor(self.cursor)
        if source.hasText():
            pasteList = QStringList()
            pasteList = source.text().split("\n")
            # with multi-line text also run the commands
            for line in pasteList[:-1]:
              self.insertPlainText(line)
              self.runCommand(unicode(self.currentCommand()))
            # last line: only paste the text, do not run it
            self.insertPlainText(unicode(pasteList[-1]))

  def entered(self):
    self.cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
    self.setTextCursor(self.cursor)
    self.runCommand( unicode(self.currentCommand()) )

  def insertTaggedText(self, txt, tag):

    if len(txt) > 0 and txt[-1] == '\n': # remove trailing newline to avoid one more empty line
      txt = txt[0:-1]

    c = self.textCursor()
    for line in txt.split('\n'):
      b = c.block()
      b.setUserState(tag)
      c.insertText(line)
      c.insertBlock()

  def insertTaggedLine(self, txt, tag):
    c = self.textCursor()
    b = c.block()
    b.setUserState(tag)
    c.insertText(txt)

  def runCommand(self, cmd):

    self.updateHistory(cmd)

    self.insertPlainText("\n")

    self.buffer.append(cmd)
    src = "\n".join(self.buffer)
    more = self.runsource(src, "<input>")
    if not more:
      self.buffer = []

    output = sys.stdout.get_and_clean_data()
    if output:
      self.insertTaggedText(output, ConsoleHighlighter.OUTPUT)
    self.displayPrompt(more)

  def write(self, txt):
    """ reimplementation from code.InteractiveInterpreter """
    self.insertTaggedText(txt, ConsoleHighlighter.ERROR)

if __name__ == '__main__':
  a = QApplication(sys.argv)
  show_console()
  a.exec_()
