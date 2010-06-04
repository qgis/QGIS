# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetProjection import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from dialogSRS import GdalToolsSRSDialog as SRSDialog
import GdalTools_utils as Utils

import os.path

class GdalToolsBatchProjectionWidget(BaseBatchWidget):
  def __init__(self, iface, commandName, helpFileBaseName = None):
      BaseBatchWidget.__init__(self, iface, commandName, helpFileBaseName)

      layerMap = QgsMapLayerRegistry.instance().mapLayers()
      self.layerList = []
      for name, layer in layerMap.iteritems():
        if layer.type() == QgsMapLayer.RasterLayer:
          self.layerList.append( unicode( layer.name() ) )

  def batchRun(self):
      self.base.enableRun( False )
      self.base.setCursor( Qt.WaitCursor )

      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()

      filter = Utils.getRasterExtensions()
      self.inFiles = []
      self.outFiles = []

      if self.recurseCheck.isChecked():
        workDir = QDir( inDir )
        workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
        workDir.setNameFilters( filter )
        workFiles = workDir.entryList()
        for f in workFiles:
          self.inFiles.append( QString( inDir + "/" + f ) )
        for myRoot, myDirs, myFiles in os.walk( str( inDir ) ):
          for dir in myDirs:
            workDir = QDir( myRoot + "/" + dir )
            workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
            workDir.setNameFilters( filter )
            workFiles = workDir.entryList()
            for f in workFiles:
              self.inFiles.append( QString( myRoot + "/" + dir + "/" + f ) )
        for f in self.inFiles:
          if outDir != None:
            outFile = QString( f ).insert( f.indexOf( "." ), "_1" ).replace( QRegExp( "\.[a-zA-Z]{3,4}$" ), ".tif" )
            self.outFiles.append( outFile )
      else:
        workDir = QDir( inDir )
        workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
        workDir.setNameFilters( filter )
        files = workDir.entryList()
        for f in files:
          self.inFiles.append( inDir + "/" + f )
          if outDir != None:
            outFile = QString( f ).insert( f.indexOf( "." ), "_1" ).replace( QRegExp( "\.[a-zA-Z]{3,4}$" ), ".tif" )
            self.outFiles.append( outDir + "/" + outFile )

      #files = QStringList()
      #for myRoot, myDirs, myFiles in os.walk( str( inDir ) ):
      #  for dir in myDirs:
      #    print "DIR", dir
      #    workDir = QDir( myRoot + "/" + dir )
      #    workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      #    workDir.setNameFilters( filter )
      #    workFiles = workDir.entryList()
      #    print "ENTRY", workFiles.join( " " )
      #    for f in workFiles:
      #      files.append( QString( myRoot + "/" + dir + "/" + f ) )

      #print "FILES", files.join( "\n" )

      #filter = Utils.getRasterExtensions()
      #workDir = QDir( inDir )
      #workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      #workDir.setNameFilters( filter )
      #files = workDir.entryList()

      #self.inFiles = []
      #self.outFiles = []

      #for f in files:
      #  self.inFiles.append( inDir + "/" + f )
      #  if outDir != None:
      #    outFile = QString( f ).insert( f.indexOf( "." ), "_1" ).replace( QRegExp( "\.[a-zA-Z]{3,4}$" ), ".tif" )
      #    self.outFiles.append( outDir + "/" + outFile )

      self.errors = QStringList()
      self.batchIndex = 0
      self.batchTotal = len( self.inFiles )
      self.setProgressRange( self.batchTotal )

      self.runItem( self.batchIndex, self.batchTotal )

  def runItem(self, index, total):
      if index >= total:
        self.outFiles = self.inFiles

      BaseBatchWidget.runItem(self, index, total)

  def onFinished(self, exitCode, status):
      if not self.isBatchEnabled():
        BaseBatchWidget.onFinished(self, exitCode, status)
        return

      oldFile = QFile( self.inFiles[self.batchIndex] )
      newFile = QFile( self.outFiles[self.batchIndex] )
      if oldFile.remove():
        newFile.rename(self.inFiles[self.batchIndex])

      BaseBatchWidget.onFinished(self, exitCode, status)


class GdalToolsDialog( QWidget, Ui_Widget, GdalToolsBatchProjectionWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface

      self.setupUi( self )
      GdalToolsBatchProjectionWidget.__init__( self, self.iface, "gdalwarp" )

      # set the default QSpinBoxes and QProgressBar value
      self.progressBar.setValue(0)

      self.progressBar.hide()
      self.recurseCheck.hide()
      # store temporary file name
      self.tempFile = QString()

      self.setParamsStatus(
        [
          ( self.inputFileEdit, SIGNAL( "textChanged( const QString & )" ) ),
          ( self.desiredSRSEdit, SIGNAL( "textChanged( const QString & )" ) )
        ]
      )

      self.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )
      self.connect( self.selectDesiredSRSButton, SIGNAL( "clicked()" ), self.fillDesiredSRSEdit )
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )
      self.connect( self.recurseCheck, SIGNAL( "stateChanged( int )" ), self.enableRecurse )

      #QObject.disconnect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.onRun )
      #QObject.connect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.checkLayer )

  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )
      self.inputFileEdit.clear()

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory:" ) )

        self.progressBar.show()
        self.recurseCheck.show()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )
        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )

        QObject.disconnect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.checkLayer )
      else:
        self.label.setText( self.inFileLabel )

        self.progressBar.hide()
        self.recurseCheck.hide()

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )
        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )

        QObject.connect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.checkLayer )

  def enableRecurse( self ):
    if self.recurseCheck.isChecked():
      res = QMessageBox.warning( self, self.tr( "Warning" ),
                                 self.tr( "Warning: CRS information for all raster in subfolders will be rewritten. Are you sure?" ),
                                 QMessageBox.Yes | QMessageBox.No )
      if res != QMessageBox.Yes:
        self.recurseCheck.setCheckState( Qt.Unchecked )
        return

  def fillInputFileEdit( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      file = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the file to analyse" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if file.isEmpty():
        return

      outFile = QString( file ).insert( file.indexOf( "." ), "_1" ).replace( QRegExp( "\.[a-zA-Z]{3,4}$" ), ".tif" )
      self.tempFile = outFile
      #self.outputFormat = Utils.fillOutputFormat( lastUsedFilter, file )
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )
      self.inputFileEdit.setText( file )

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files to Assign projection" ))
      if inputDir.isEmpty():
        return

      self.inputPath = inputDir
      self.inputFileEdit.setText( inputDir )

      filter = Utils.getRasterExtensions()
      workDir = QDir( inputDir )
      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      workDir.setNameFilters( filter )

  def fillDesiredSRSEdit( self ):
      dialog = SRSDialog( "Select desired SRS" )
      if dialog.exec_():
        self.desiredSRSEdit.setText( dialog.getProjection() )

  def getArguments( self ):
      arguments = QStringList()
      if not self.desiredSRSEdit.text().isEmpty():
        arguments << "-t_srs"
        arguments << self.desiredSRSEdit.text()
      #if not self.inputFileEdit.text().isEmpty():
      #  arguments << "-of"
      #  arguments << self.outputFormat
      if self.batchCheck.isChecked():
        return arguments
      arguments << self.inputFileEdit.text()
      arguments << self.tempFile
      return arguments

  def getInputFileName(self):
      return self.inputFileEdit.text()

  def getOutputFileName( self ):
      return self.inputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def checkLayer( self ):
      layerMap = QgsMapLayerRegistry.instance().mapLayers()
      layerList = []
      for name, layer in layerMap.iteritems():
        if layer.type() == QgsMapLayer.RasterLayer:
          layerList.append( unicode( layer.name() ) )
      fileName = QString( os.path.split( str( self.inputFileEdit.text() ) )[ 1 ] )
      fileName = fileName.left( fileName.indexOf( "." ) )
      if fileName in layerList:
        QMessageBox.warning( self, self.tr( "Assign projection" ), self.tr( "This raster already found in map canvas" ) )
        return
      self.onRun()

  def isBatchEnabled(self):
      return self.batchCheck.isChecked()

  def setProgressRange(self, maximum):
      self.progressBar.setRange(0, maximum)

  def updateProgress(self, index, total):
      if index < total:
        self.progressBar.setValue(index + 1)
      else:
        self.progressBar.setValue(0)

  def finished( self ):
      oldFile = QFile( self.inputFileEdit.text() )
      newFile = QFile( self.tempFile )
      if oldFile.remove():
        newFile.rename( self.inputFileEdit.text() )

#  def batchRun( self ):
#      self.base.buttonBox.button( QDialogButtonBox.Ok ).setEnabled( False )
#      self.base.setCursor( Qt.WaitCursor )
#
#      filter = Utils.getRasterExtensions()
#      inDir = self.inputFileEdit.text()
#      outDir = inDir
#      workDir = QDir( inDir )
#      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
#      workDir.setNameFilters( filter )
#      files = workDir.entryList()
#
#      inFiles = []
#      self.outFiles = []
#      for f in files:
#        inFiles.append( inDir + "/" + f )
#        outFile = QString( f ).insert( f.indexOf( "." ), "_1" ).replace( QRegExp( "\.[a-zA-Z]{3,4}$" ), ".tif" )
#        #outFile = f.replace( QRegExp( "\.[a-zA-Z0-9]{2,4}" ), ".tif" )
#        self.outFiles.append( outDir + "/" + outFile )
#
#      args = QStringList()
#      for i in range( self.base.arguments.count() ):
#        args.append( self.base.arguments[ i ] )
#
#      self.errors = QStringList()
#      count = 1
#      total = len( inFiles )
#      for i, item in enumerate( inFiles ):
#        #print "PROCESS", i
#        #self.lblProgress.setText( self.tr( "Processed: %1 from %2" ).arg( count ).arg( total ) )
#        #QCoreApplication.processEvents()
#        count = count + 1
#
#        itemArgs = QStringList()
#        itemArgs << args
#        itemArgs << item
#        itemArgs << self.outFiles[ i ]
#
#        self.base.process.start( self.base.command, itemArgs, QIODevice.ReadOnly )
#        if self.base.process.waitForFinished():
#          msg = QString( self.base.process.readAllStandardError() )
#          if not msg.isEmpty():
#            self.errors.append( item )
#          self.base.process.close()
#
#        oldFile = QFile( item )
#        newFile = QFile( self.outFiles[ i ] )
#        if oldFile.remove():
#          newFile.rename( item )
#
#      #self.lblProgress.setText( self.tr( "Finished" ) )
#      self.batchFinished()
#
#  def batchFinished( self ):
#      self.base.buttonBox.button( QDialogButtonBox.Ok ).setEnabled( True )
#      self.base.setCursor( Qt.ArrowCursor )
#
#      if not self.errors.isEmpty():
#        message = QErrorMessage( self )
#        msg = QString( "Processing of the following files ended with error:\n\n" ).append( self.errors.join( "\n" ) )
#        message.showMessage( msg )

