# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetWarp import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from dialogSRS import GdalToolsSRSDialog as SRSDialog
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.resampling_method = ('near', 'bilinear', 'cubic', 'cubicspline', 'lanczos')

      self.setupUi(self)
      BaseBatchWidget.__init__(self, self.iface, "gdalwarp")

      # set the default QSpinBoxes and QProgressBar value
      self.widthSpin.setValue(3000)
      self.heightSpin.setValue(3000)
      self.progressBar.setValue(0)

      self.progressBar.hide()

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.sourceSRSEdit, SIGNAL("textChanged(const QString &)"), self.sourceSRSCheck),
          (self.selectSourceSRSButton, None, self.sourceSRSCheck),
          (self.targetSRSEdit, SIGNAL("textChanged(const QString &)"), self.targetSRSCheck),
          (self.selectTargetSRSButton, None, self.targetSRSCheck),
          (self.resamplingCombo, SIGNAL("currentIndexChanged(int)"), self.resamplingCheck),
          (self.cacheSpin, SIGNAL("valueChanged(int)"), self.cacheCheck),
          ( [self.widthSpin, self.heightSpin], SIGNAL( "valueChanged(int)" ), self.resizeGroupBox ),
          (self.multithreadCheck, SIGNAL("stateChanged(int)")),
          (self.noDataEdit, SIGNAL( "textChanged( const QString & )" ), self.noDataCheck)
        ]
      )

      self.connect(self.inputLayerCombo, SIGNAL("currentIndexChanged(int)"), self.fillSourceSRSEditDefault)
      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFile)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.selectSourceSRSButton, SIGNAL("clicked()"), self.fillSourceSRSEdit)
      self.connect(self.selectTargetSRSButton, SIGNAL("clicked()"), self.fillTargetSRSEdit)
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )


  # switch to batch or normal mode
  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )

      self.inputLayerCombo.clear()
      self.inputLayerCombo.clearEditText()
      self.inputLayerCombo.setCurrentIndex(-1)
      self.outputFileEdit.clear()

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.outFileLabel = self.label_2.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )
        self.label_2.setText( QCoreApplication.translate( "GdalTools", "&Output directory" ) )

        self.progressBar.show()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFile )
        QObject.disconnect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputFileEdit )

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )
        QObject.connect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputDir )
      else:
        self.label.setText( self.inFileLabel )
        self.label_2.setText( self.outFileLabel )

        self.fillInputLayerCombo()

        self.progressBar.hide()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )
        QObject.disconnect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputDir )

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFile )
        QObject.connect( self.selectOutputFileButton, SIGNAL( "clicked()" ), self.fillOutputFileEdit )

  def onLayersChanged(self):
      self.fillInputLayerCombo()

  def fillInputLayerCombo(self):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.LayerRegistry.instance().getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFile(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Warp" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)

      # get SRS for source file if necessary and possible
      self.refreshSourceSRS()

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outputFileEdit.setText(outputFile)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files to Warp" ))
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
        self.sourceSRSEdit.setText( Utils.getRasterSRS( self, fl ) )

  def fillOutputDir( self ):
      outputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the output directory to save the results to" ))
      if outputDir.isEmpty():
        return

      self.outputFileEdit.setText( outputDir )

  def fillSourceSRSEdit(self):
      dialog = SRSDialog( "Select the source SRS" )
      if dialog.exec_():
        self.sourceSRSEdit.setText(dialog.getProjection())

  def fillSourceSRSEditDefault(self, index):
      if index < 0:
        return
      self.refreshSourceSRS()

  def refreshSourceSRS(self):
      crs = Utils.getRasterSRS( self, self.getInputFileName() )
      self.sourceSRSEdit.setText( crs )
      self.sourceSRSCheck.setChecked( not crs.isEmpty() )

  def fillTargetSRSEdit(self):
      dialog = SRSDialog( "Select the target SRS" )
      if dialog.exec_():
        self.targetSRSEdit.setText(dialog.getProjection())

  def getArguments(self):
      arguments = QStringList()
      if self.sourceSRSCheck.isChecked() and not self.sourceSRSEdit.text().isEmpty():
        arguments << "-s_srs"
        arguments << self.sourceSRSEdit.text()
      if self.targetSRSCheck.isChecked() and not self.targetSRSEdit.text().isEmpty():
        arguments << "-t_srs"
        arguments << self.targetSRSEdit.text()
      if self.resamplingCheck.isChecked() and self.resamplingCombo.currentIndex() >= 0:
        arguments << "-r"
        arguments << self.resampling_method[self.resamplingCombo.currentIndex()]
      if self.cacheCheck.isChecked():
        arguments << "-wm"
        arguments << str(self.cacheSpin.value())
      if self.resizeGroupBox.isChecked():
        arguments << "-ts"
        arguments << str( self.widthSpin.value() )
        arguments << str( self.heightSpin.value() )
      if self.multithreadCheck.isChecked():
        arguments << "-multi"
      if self.noDataCheck.isChecked():
        nodata = self.noDataEdit.text().trimmed()
        if not nodata.isEmpty():
          arguments << "-dstnodata"
          arguments << nodata
      if self.isBatchEnabled():
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
