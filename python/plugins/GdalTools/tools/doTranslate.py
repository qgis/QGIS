# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetTranslate import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from dialogSRS import GdalToolsSRSDialog as SRSDialog
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.canvas = self.iface.mapCanvas()

      self.setupUi(self)
      BaseBatchWidget.__init__(self, self.iface, "gdal_translate")

      # set the default QSpinBoxes and QProgressBar value
      self.progressBar.setValue(0)

      self.progressBar.hide()
      self.formatLabel.hide()
      self.formatCombo.hide()

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.targetSRSEdit, SIGNAL("textChanged(const QString &)"), self.targetSRSCheck),
          (self.selectTargetSRSButton, None, self.targetSRSCheck),
          (self.creationOptionsTable, [SIGNAL("cellValueChanged(int, int)"), SIGNAL("rowRemoved()")], self.creationGroupBox)
        ]
      )

      #self.connect(self.canvas, SIGNAL("layersChanged()"), self.fillInputLayerCombo)
      self.connect(self.inputLayerCombo, SIGNAL("currentIndexChanged(int)"), self.fillTargetSRSEditDefault)
      self.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFile )
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.selectTargetSRSButton, SIGNAL("clicked()"), self.fillTargetSRSEdit)
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

      # add raster filters to combo
      self.formatCombo.addItems( Utils.FileFilter.allRastersFilter().split( ";;" ) )

      # add layers to combo
      self.fillInputLayerCombo()

  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )

      self.inputLayerCombo.clear()
      self.inputLayerCombo.clearEditText()
      self.inputLayerCombo.setCurrentIndex(-1)
      self.outputFileEdit.clear()

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label_3.text()
        self.outFileLabel = self.label_2.text()
        self.label_3.setText( QCoreApplication.translate( "GdalTools", "&Input directory:" ) )
        self.label_2.setText( QCoreApplication.translate( "GdalTools", "&Output directory:" ) )

        self.progressBar.show()
        self.formatLabel.show()
        self.formatCombo.show()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFile )
        QObject.disconnect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputFileEdit )

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self. fillInputDir )
        QObject.connect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputDir )
      else:
        self.label_3.setText( self.inFileLabel )
        self.label_2.setText( self.outFileLabel )
        self.base.textEditCommand.setEnabled( True )
        self.fillInputLayerCombo()

        self.progressBar.hide()
        self.formatLabel.hide()
        self.formatCombo.hide()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )
        QObject.disconnect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputDir )

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFile )
        QObject.connect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputFileEdit )

  def fillInputLayerCombo(self):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the input file for Translate" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      # get SRS for target file if necessary and possible
      self.targetSRSEdit.setText( Utils.getRasterSRS( self, inputFile ) )

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText( inputFile )

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input direcory with files to Translate" ))
      if inputDir.isEmpty():
        return

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText( inputDir )

      filter = Utils.getRasterExtensions()
      workDir = QDir( inputDir )
      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      workDir.setNameFilters( filter )
      if workDir.entryList().count() > 0:
        fl = inputDir + "/" + workDir.entryList()[ 0 ]
        self.targetSRSEdit.setText( Utils.getRasterSRS( self, fl ) )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
      self.outputFileEdit.setText(outputFile)

  def fillOutputDir( self ):
      outputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the output directory to save the results to" ) )
      if outputDir.isEmpty():
        return

      self.outputFileEdit.setText( outputDir )

  def fillTargetSRSEditDefault(self, index):
      if index >= 0 and index in self.layers:
        layer = self.layers[index]
        self.targetSRSEdit.setText("EPSG:" + str(layer.srs().epsg()))

  def fillTargetSRSEdit(self):
      dialog = SRSDialog( "Select the target SRS" )
      if dialog.exec_():
        self.targetSRSEdit.setText(dialog.getProjection())

  def getArguments(self):
      arguments = QStringList()
      if self.targetSRSCheck.isChecked() and not self.targetSRSEdit.text().isEmpty():
        arguments << "-a_srs"
        arguments << self.targetSRSEdit.text()
      if self.creationGroupBox.isChecked():
        for opt in self.creationOptionsTable.options():
          arguments << "-co"
          arguments << opt
      if self.isBatchEnabled():
        if self.formatCombo.currentIndex() != 0:
          arguments << "-of"
          arguments << Utils.fillRasterOutputFormat( self.formatCombo.currentText() )
          return arguments
        else:
          return arguments

      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << self.getOutputFileName()
      return arguments

  def getInputFileName(self):
      if self.inputLayerCombo.currentIndex() >= 0:
        return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def getOutputFileName(self):
      return self.outputFileEdit.text()

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
          outFile = f.replace( QRegExp( "\.[a-zA-Z0-9]{2,4}" ), outExt )
          self.outFiles.append( outDir + "/" + outFile )

      self.errors = QStringList()
      self.batchIndex = 0
      self.batchTotal = len( self.inFiles )
      self.setProgressRange( self.batchTotal )

      self.runItem( self.batchIndex, self.batchTotal )
