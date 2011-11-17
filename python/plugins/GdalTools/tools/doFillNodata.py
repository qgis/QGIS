# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetFillNodata import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
import GdalTools_utils as Utils

import os.path

class GdalToolsDialog( QWidget, Ui_Widget, BaseBatchWidget ):
  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface

      self.setupUi( self )
      BaseBatchWidget.__init__( self, self.iface, "gdal_fillnodata.py" )

      self.inSelector.setType( self.inSelector.FILE_LAYER )
      self.outSelector.setType( self.outSelector.FILE )
      self.maskSelector.setType( self.maskSelector.FILE )

      self.progressBar.setValue(0)
      self.progressBar.hide()
      self.formatLabel.hide()
      self.formatCombo.hide()

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          ( self.inSelector, SIGNAL( "filenameChanged()" ) ),
          ( self.outSelector, SIGNAL( "filenameChanged()" ) ),
          ( self.maskSelector, SIGNAL( "filenameChanged()" ), self.maskCheck ),
          ( self.distanceSpin, SIGNAL( "valueChanged( int )" ), self.distanceCheck ),
          ( self.smoothSpin, SIGNAL( "valueChanged( int )" ), self.smoothCheck ),
          ( self.bandSpin, SIGNAL( "valueChanged( int )" ), self.bandCheck ),
          ( self.nomaskCheck, SIGNAL( "stateChanged( int )" ) )
        ]
      )

      self.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
      self.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFile)
      self.connect( self.maskSelector, SIGNAL( "selectClicked()" ), self.fillMaskFile)
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

      # add raster filters to combo
      self.formatCombo.addItems( Utils.FileFilter.allRastersFilter().split( ";;" ) )


  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )
      self.progressBar.setVisible( self.batchCheck.isChecked() )
      self.formatLabel.setVisible( self.batchCheck.isChecked() )
      self.formatCombo.setVisible( self.batchCheck.isChecked() )

      self.inSelector.setType( self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER )
      self.outSelector.clear()

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.outFileLabel = self.label_1.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )
        self.label_1.setText( QCoreApplication.translate( "GdalTools", "&Output directory" ) )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.disconnect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFile )

        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self. fillInputDir )
        QObject.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputDir )
      else:
        self.label.setText( self.inFileLabel )
        self.label_1.setText( self.outFileLabel )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
        QObject.disconnect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputDir )

        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFile )

  def fillInputFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self,
                                                    self.tr( "Select the files to analyse" ),
                                                    Utils.FileFilter.allRastersFilter(),
                                                    lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )
      self.inSelector.setFilename( inputFile )

  def fillOutputFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName( self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename( outputFile )

  def fillMaskFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self,
                                                    self.tr( "Select the files to analyse" ),
                                                    Utils.FileFilter.allRastersFilter(),
                                                    lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )
      self.maskSelector.setFilename( inputFile )

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files" ))
      if inputDir.isEmpty():
        return
      self.inSelector.setFilename( inputDir )

  def fillOutputDir( self ):
      outputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the output directory to save the results to" ) )
      if outputDir.isEmpty():
        return
      self.outSelector.setFilename( outputDir )

  def getArguments(self):
      arguments = QStringList()
      maskFile = self.maskSelector.filename()
      if self.distanceCheck.isChecked() and self.distanceSpin.value() != 0:
        arguments << "-md"
        arguments << self.distanceSpin.text()
      if self.smoothCheck.isChecked() and self.smoothSpin.value() != 0:
        arguments << "-si"
        arguments << str( self.smoothSpin.value() )
      if self.bandCheck.isChecked() and self.bandSpin.value() != 0:
        arguments << "-b"
        arguments << str( self.bandSpin.value() )
      if self.maskCheck.isChecked() and not maskFile.isEmpty():
        arguments << "-mask"
        arguments << maskFile
      if self.nomaskCheck.isChecked():
        arguments << "-nomask"
      if self.isBatchEnabled():
        if self.formatCombo.currentIndex() != 0:
          arguments << "-of"
          arguments << Utils.fillRasterOutputFormat( self.formatCombo.currentText() )
        return arguments
      else:
        outputFn = self.getOutputFileName()
        if not outputFn.isEmpty():
          arguments << "-of"
          arguments << self.outputFormat
        arguments << self.getInputFileName()
        arguments << outputFn
        return arguments

  def onLayersChanged( self ):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def getInputFileName(self):
      return self.inSelector.filename()

  def getOutputFileName(self):
      return self.outSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def isBatchEnabled(self):
      return self.batchCheck.isChecked()

  def setProgressRange(self, maximum):
      self.progressBar.setRange(0, maximum)

  def updateProgress(self, index, total):
      if index < total:
        self.progressBar.setValue( index + 1 )
      else:
        self.progressBar.setValue( 0 )

  def batchRun(self):
      exts = self.formatCombo.currentText().remove( QRegExp('^.*\(') ).remove( QRegExp('\).*$') ).split( " " )
      if not exts.isEmpty() and exts != "*" and exts != "*.*":
        outExt = exts[ 0 ].remove( "*" )
      else:
        outExt = ".tif"

      self.base.enableRun( False )
      self.base.setCursor( Qt.WaitCursor )

      inDir = self.getInputFileName()
      outDir = self.getOutputFileName()

      extensions = Utils.getRasterExtensions()
      workDir = QDir( inDir )
      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      workDir.setNameFilters( extensions )
      files = workDir.entryList()

      self.inFiles = []
      self.outFiles = []

      for f in files:
        self.inFiles.append( inDir + "/" + f )
        if outDir != None:
          outFile = f.replace( QRegExp( "\.[a-zA-Z0-9]{2,4}" ), outExt )
          self.outFiles.append( outDir + "/" + outFile )

      self.errors = QStringList()
      self.batchIndex = 0
      self.batchTotal = len( self.inFiles )
      self.setProgressRange( self.batchTotal )

      self.runItem( self.batchIndex, self.batchTotal )
