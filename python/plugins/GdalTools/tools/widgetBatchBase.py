# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsBaseBatchWidget(BasePluginWidget):

  def __init__(self, iface, commandName, helpFileBaseName = None):
      BasePluginWidget.__init__(self, iface, commandName, helpFileBaseName)

  def getBatchArguments(self, inFile, outFile = None):
      arguments = QStringList()
      arguments << self.getArguments()
      arguments << inFile
      if outFile != None:
        arguments << outFile
      return arguments

  def isBatchEnabled(self):
      return False

  def setProgressRange(self, maximum ):
      pass

  def updateProgress(self, value, maximum):
      pass

  def onRun( self ):
      if not self.isBatchEnabled():
        BasePluginWidget.onRun(self)
        return

      self.batchRun()

  def batchRun(self):
      self.base.enableRun( False )
      self.base.setCursor( Qt.WaitCursor )

      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()

      filter = Utils.getRasterExtensions()
      workDir = QDir( inDir )
      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      workDir.setNameFilters( filter )
      files = workDir.entryList()

      self.inFiles = []
      self.outFiles = []

      for f in files:
        self.inFiles.append( inDir + "/" + f )
        if outDir != None:
          outFile = f.replace( QRegExp( "\.[a-zA-Z0-9]{2,4}" ), ".tif" )
          self.outFiles.append( outDir + "/" + outFile )

      self.errors = QStringList()
      self.batchIndex = 0
      self.batchTotal = len( self.inFiles )
      self.setProgressRange( self.batchTotal )

      self.runItem( self.batchIndex, self.batchTotal )

  def runItem(self, index, total):
      self.updateProgress(index, total)

      if index >= total:
        self.batchFinished()
        return

      outFile = None
      if len(self.outFiles) > index:
        outFile = self.outFiles[ index ]

      args = self.getBatchArguments( self.inFiles[index], outFile )
      self.base.refreshArgs(args)
      BasePluginWidget.onRun(self)

  def onFinished(self, exitCode, status):
      if not self.isBatchEnabled():
        BasePluginWidget.onFinished(self, exitCode, status)
        return

      msg = QString( self.base.process.readAllStandardError() )
      if not msg.isEmpty():
        self.errors.append( ">> " + self.inFiles[self.batchIndex] + "<br>" + msg.replace( "\n", "<br>" ) )

      self.base.process.close()

      self.batchIndex += 1
      self.runItem( self.batchIndex, self.batchTotal )

  def batchFinished( self ):
      self.base.stop()

      if not self.errors.isEmpty():
        msg = QString( "Processing of the following files ended with error: <br><br>" ).append( self.errors.join( "<br><br>" ) )
        QErrorMessage( self ).showMessage( msg )

      notCreatedList = QStringList()
      for item in self.outFiles:
        fileInfo = QFileInfo( item )
        if fileInfo.exists():
          if self.base.loadCheckBox.isChecked():
            self.addLayerIntoCanvas( fileInfo )
        else:
          notCreatedList << item

      if notCreatedList.isEmpty():
        QMessageBox.information( self, self.tr( "Finished" ), self.tr( "Operation completed." ) )
      else:
        QMessageBox.warning( self, self.tr( "Warning" ), self.tr( "The following files were not created: \n%1" ).arg( notCreatedList.join( ", " ) ) )

