# -*- coding: utf-8 -*-

"""
***************************************************************************
    widgetBatchBase.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsBaseBatchWidget(BasePluginWidget):

  def __init__(self, iface, commandName):
      BasePluginWidget.__init__(self, iface, commandName)

  def getBatchArguments(self, inFile, outFile = None):
      arguments = QStringList()
      arguments << self.getArguments()
      arguments << inFile
      if outFile != None:
        arguments << outFile
      return arguments

  def isBatchEnabled(self):
      return False

  def isRecursiveScanEnabled(self):
      return False

  def setProgressRange(self, maximum):
      pass

  def updateProgress(self, value, maximum):
      pass

  def getBatchOutputFileName(self, fn):
      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()

      # if overwrites existent files
      if outDir == None or outDir == inDir:
        return QString( fn ).append( ".tmp" )

      return QString( fn ).mid( len(inDir) ).prepend( outDir )

  def onRun( self ):
      if not self.isBatchEnabled():
        BasePluginWidget.onRun(self)
        return

      self.batchRun()

  def batchRun(self):
      self.base.enableRun( False )
      self.base.setCursor( Qt.WaitCursor )

      inDir = self.getInputFileName()

      self.inFiles = Utils.getRasterFiles( inDir, self.isRecursiveScanEnabled() )
      self.outFiles = []

      for f in self.inFiles:
        self.outFiles.append( self.getBatchOutputFileName( f ) )

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

      msg = QString.fromLocal8Bit( self.base.process.readAllStandardError() )
      if not msg.isEmpty():
        self.errors.append( ">> " + self.inFiles[self.batchIndex] + "<br>" + msg.replace( "\n", "<br>" ) )

      self.base.process.close()

      # overwrite existent files
      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()
      if outDir == None or inDir == outDir:
        oldFile = QFile( self.inFiles[self.batchIndex] )
        newFile = QFile( self.outFiles[self.batchIndex] )
        if oldFile.remove():
          newFile.rename(self.inFiles[self.batchIndex])

      self.batchIndex += 1
      self.runItem( self.batchIndex, self.batchTotal )

  def batchFinished( self ):
      self.base.stop()

      if not self.errors.isEmpty():
        msg = QString( "Processing of the following files ended with error: <br><br>" ).append( self.errors.join( "<br><br>" ) )
        QErrorMessage( self ).showMessage( msg )

      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()
      if outDir == None or inDir == outDir:
        self.outFiles = self.inFiles

      # load layers managing the render flag to avoid waste of time
      canvas = self.iface.mapCanvas()
      previousRenderFlag = canvas.renderFlag()
      canvas.setRenderFlag( False )
      notCreatedList = QStringList()
      for item in self.outFiles:
        fileInfo = QFileInfo( item )
        if fileInfo.exists():
          if self.base.loadCheckBox.isChecked():
            self.addLayerIntoCanvas( fileInfo )
        else:
          notCreatedList << item
      canvas.setRenderFlag( previousRenderFlag )

      if notCreatedList.isEmpty():
        QMessageBox.information( self, self.tr( "Finished" ), self.tr( "Operation completed." ) )
      else:
        QMessageBox.warning( self, self.tr( "Warning" ), self.tr( "The following files were not created: \n%1" ).arg( notCreatedList.join( ", " ) ) )

