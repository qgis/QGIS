# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

# to know the os
import platform
import os

from ui_dialogBase import Ui_GdalToolsDialog as Ui_Dialog
import GdalTools_utils as Utils

import os, platform

class GdalToolsBaseDialog(QDialog, Ui_Dialog):

  def __init__(self, parent, iface, pluginBase, pluginName, pluginCommand):
      QDialog.__init__(self, parent)
      self.setAttribute(Qt.WA_DeleteOnClose)
      self.iface = iface

      self.process = QProcess(self)
      gdalPath = Utils.getGdalPath()
      if not gdalPath.isEmpty():
          env = self.process.environment()
          if env.isEmpty():
            #env << "PATH=" + gdalPath
            os.putenv( "PATH", str( gdalPath ) )
          else:
            if platform.system() == "Windows":
              env.replaceInStrings( QRegExp( "^PATH=(.*)", Qt.CaseInsensitive ), "PATH=\\1;" + gdalPath )
            else:
              env.replaceInStrings( QRegExp( "^PATH=(.*)", Qt.CaseInsensitive ), "PATH=\\1:" + gdalPath )
          self.process.setEnvironment( env )
      self.connect(self.process, SIGNAL("error(QProcess::ProcessError)"), self.processError)
      self.connect(self.process, SIGNAL("finished(int, QProcess::ExitStatus)"), self.processFinished)

      self.setupUi(self)

      self.connect(self.buttonBox, SIGNAL("rejected()"), self.reject)
      self.connect(self.buttonBox, SIGNAL("accepted()"), self.accept)
      self.connect(self.buttonBox, SIGNAL("helpRequested()"), self.help)

      self.buttonBox.button(QDialogButtonBox.Ok).setDefault(True)

      self.plugin = pluginBase
      self.connect(self.plugin, SIGNAL("valuesChanged(const QStringList &)"), self.refreshArgs)

      self.pluginLayout.addWidget(self.plugin)
      self.plugin.setFocus()

      self.setWindowTitle(pluginName)

      # on Windows replace the .py with .bat extension
      if platform.system() == "Windows" and pluginCommand[-3:] == ".py":
        self.command = pluginCommand[:-3] + ".bat"
      else:
        self.command = pluginCommand

      if pluginCommand[-3:] == ".py":
        self.helpFileName = pluginCommand[:-3] + ".html"
      else:
        self.helpFileName = pluginCommand + ".html"

      self.arguments = QStringList()

  def reject(self):
      self.emit( SIGNAL("closeClicked()") )

  def accept(self):
      self.emit( SIGNAL("okClicked()") )

  def help(self):
      self.emit( SIGNAL("helpClicked()") )

  def processError(self, error):
      self.emit( SIGNAL("processError(QProcess::ProcessError)"), error )

  def processFinished(self, exitCode, status):
      self.emit( SIGNAL("processFinished(int, QProcess::ExitStatus)"), exitCode, status )

  # show the online tool documentation in the default browser
  def onHelp(self):
      helpPath = Utils.getHelpPath()
      if helpPath.isEmpty():
        url = QUrl("http://www.gdal.org/" + self.helpFileName)
      else:
        url = QUrl.fromLocalFile(helpPath + '/' + self.helpFileName)
      QDesktopServices.openUrl(url)

  def setCommandViewerEnabled(self, enable):
      self.textEditCommand.setEnabled( enable )

  # called when a value in the plugin widget interface changed
  def refreshArgs(self, args):
      self.arguments = args

      if not self.textEditCommand.isEnabled():
        self.textEditCommand.setText(self.command)
      else:
        self.textEditCommand.setText(self.command + " " + Utils.escapeAndJoin(self.arguments))

  # enables the OK button
  def enableRun(self, enable = True):
      self.buttonBox.button(QDialogButtonBox.Ok).setEnabled(enable)

  # start the command execution
  def onRun(self):
      self.process.start(self.command, self.arguments, QIODevice.ReadOnly)
      self.enableRun(False)
      self.setCursor(Qt.WaitCursor)

      """
      try:
        print "Debug: " + self.command + " " + unicode(Utils.escapeAndJoin(self.arguments))
      except UnicodeEncodeError:
        try:
          print "Debug: " + self.command + " " + unicode(Utils.escapeAndJoin(self.arguments)).encode('cp866', 'replace')
        except:
          print "Debug: " + self.command + " " + unicode(Utils.escapeAndJoin(self.arguments)).encode('ascii', 'replace')
      """

  # stop the command execution
  def stop(self):
      self.enableRun(True)
      self.setCursor(Qt.ArrowCursor)
      self.process.kill()

  # called on closing the dialog, stop the process if it's running
  def onClosing(self):
      if self.process.state() != QProcess.NotRunning:
        ret = QMessageBox.warning(self, self.tr( "Warning" ), self.tr( "The command is still running. \nDo you want terminate it anyway?" ), QMessageBox.Yes | QMessageBox.No)
        if ret == QMessageBox.No:
          return

        self.disconnect(self.process, SIGNAL("error(QProcess::ProcessError)"), self.processError)
        self.disconnect(self.process, SIGNAL("finished(int, QProcess::ExitStatus)"), self.processFinished)
        #self.process.kill()
        #print "Debug: " + self.command + " terminated by user"

      self.stop()
      QDialog.reject(self)

  # called if an error occurs when the command has not already finished, shows the occurred error message
  def onError(self, error):
      if error == QProcess.FailedToStart:
        msg = QCoreApplication.translate( "GdalTools", "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program." )
      elif error == QProcess.Crashed:
        msg = QCoreApplication.translate( "GdalTools", "The process crashed some time after starting successfully.")
      else:
        msg = QCoreApplication.translate( "GdalTools", "An unknown error occurred.")

      QErrorMessage(self).showMessage(msg)
      QApplication.processEvents() # give the user chance to see the message

      self.stop()

  # called when the command finished its execution, shows an error message if there's one
  # and, if required, load the output file in canvas
  def onFinished(self, exitCode, status):
      if status == QProcess.CrashExit:
        self.stop()
        return

      if self.command.find( "gdalinfo" ) != -1 and exitCode == 0:
        print "*** DEBUG ***"
        self.emit( SIGNAL("finished(bool)"), self.loadCheckBox.isChecked() )
        self.stop()
        return

      # show the error message if there's one, otherwise show the process output message
      msg = QString(self.process.readAllStandardError())
      if msg.isEmpty():
        outMessages = QString(self.process.readAllStandardOutput()).split( "\n" )

        # make sure to not show the help
        for m in outMessages:
          m = m.trimmed()
          if m.isEmpty():
            continue
          if m.contains( QRegExp( "^(?:[Uu]sage:\\s)?" + QRegExp.escape(self.command) + "\\s" ) ):
            if msg.isEmpty():
              msg = self.tr ( "Invalid parameters." )
            break
          if m.contains( QRegExp( "0(?:\\.+[1-9]0{1,2})+" ) ):
            continue

          if not msg.isEmpty():
            msg += "\n"
          msg += m

      QErrorMessage(self).showMessage( msg.replace( "\n", "<br>" ) )

      if exitCode == 0:
        self.emit( SIGNAL("finished(bool)"), self.loadCheckBox.isChecked() )

      self.stop()
