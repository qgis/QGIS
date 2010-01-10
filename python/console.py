# -*- coding: utf-8 -*-

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import traceback
import sys

from ui_qgspythondialog import Ui_QgsPythonDialog

_console = None


def show_console():
  """ called from QGIS to open the console """
  global _console
  if _console is None:
    _console = QgsPythonConsole()
  _console.show()
  _console.raise_()
  _console.setWindowState( _console.windowState() & ~Qt.WindowMinimized )
  _console.activateWindow()



_old_stdout = sys.stdout

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
  
def installConsoleHook():
  sys.stdout = QgisOutputCatcher()

def uninstallConsoleHook():
  sys.stdout = _old_stdout



class ConsoleHistory(object):
  def __init__(self):
    self.cmds = []
    self.pos = 0
    self.active_cmd = u"" # active, not yet entered command

  def add_cmd(self, command):
    if len(command) != 0:
      self.cmds.append(command)
      self.pos = len(self.cmds)
      self.active_cmd = u""

  def get_previous_cmd(self, current):
    if self.pos == 0:
      return None

    if self.pos == len(self.cmds):
      self.active_cmd = current
    else:
      self.cmds[self.pos] = current

    self.pos -= 1
    return self.cmds[self.pos]

  def get_next_cmd(self, current):

    # if we're at active (last) command, don't move
    if self.pos == len(self.cmds):
      return None
      
    self.cmds[self.pos] = current
    self.pos += 1

    if self.pos == len(self.cmds):
      return self.active_cmd
    else:
      return self.cmds[self.pos]



class QgsPythonConsole(QWidget, Ui_QgsPythonDialog):
  def __init__(self, parent=None):
    QWidget.__init__(self, parent)

    self.setupUi(self)

    # minimize button was not enabled on mac
    self.setWindowFlags( self.windowFlags() | Qt.WindowMinimizeButtonHint )

    self.history = ConsoleHistory()

    self.console_globals = {}
  
  def escapeHtml(self, text):
    return text.replace("<", "&lt;").replace(">", "&gt;")

  @pyqtSlot()
  def on_pbnPrev_clicked(self):
    cmd = self.history.get_previous_cmd( self.getCommand() )
    if cmd is not None:
      self.edtCmdLine.setText(cmd)

  @pyqtSlot()
  def on_pbnNext_clicked(self):
    cmd = self.history.get_next_cmd( self.getCommand() )
    if cmd is not None:
      self.edtCmdLine.setText(cmd)

  def getCommand(self):
    return unicode(self.edtCmdLine.toPlainText())

  def execute(self, single):
    command = self.getCommand()

    self.history.add_cmd(command)

    try:
      # run command
      code = compile(command, '<console>', 'single' if single else 'exec')
      res = eval(code, self.console_globals)
      result = unicode( res ) if res is not None else u''
      
      # get standard output
      output = sys.stdout.get_and_clean_data()

      #_old_stdout.write("cmd: '"+command+"'\n")
      #_old_stdout.write("res: '"+result+"'\n")
      #_old_stdout.write("out: '"+output+"'\n")
      #_old_stdout.flush()

      # escape the result so python objects display properly and
      # we can still use html output to get nicely formatted display
      output = self.escapeHtml( output ) + self.escapeHtml( result )
      if len(output) != 0:
        output += "<br>"

    except Exception, e:
      #lst = traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_traceback)
      lst = traceback.format_exception_only(sys.exc_type, sys.exc_value)
      errorText = "<br>".join(map(self.escapeHtml, lst))
      output = "<font color=\"red\">" + errorText + "</font><br>"

    s = "<b><font color=\"green\">&gt;&gt;&gt;</font> " + self.escapeHtml( command ) + "</b><br>" + output
    self.edtCmdLine.setText("")

    self.txtHistory.moveCursor(QTextCursor.End)
    self.txtHistory.insertHtml(s)
    self.txtHistory.moveCursor(QTextCursor.End)
    self.txtHistory.ensureCursorVisible()

  @pyqtSlot()
  def on_pbnExecute_clicked(self):
    self.execute(False)

  @pyqtSlot()
  def on_pbnEval_clicked(self):
    self.execute(True)

  def showEvent(self, event):
    QWidget.showEvent(self, event)
    installConsoleHook()

  def closeEvent(self, event):
    uninstallConsoleHook()
    QWidget.closeEvent(self, event)


if __name__ == '__main__':
  import sys
  a = QApplication(sys.argv)
  w = QgsPythonConsole()
  w.show()
  a.exec_()
