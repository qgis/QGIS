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

      self.outSelector.setType( self.outSelector.FILE )

      # set the default QSpinBoxes and QProgressBar value
      self.widthSpin.setValue(3000)
      self.heightSpin.setValue(3000)
      self.progressBar.setValue(0)

      self.progressBar.hide()

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.sourceSRSEdit, SIGNAL("textChanged(const QString &)"), self.sourceSRSCheck),
          (self.selectSourceSRSButton, None, self.sourceSRSCheck),
          (self.targetSRSEdit, SIGNAL("textChanged(const QString &)"), self.targetSRSCheck),
          (self.selectTargetSRSButton, None, self.targetSRSCheck),
          (self.resamplingCombo, SIGNAL("currentIndexChanged(int)"), self.resamplingCheck),
          (self.cacheSpin, SIGNAL("valueChanged(int)"), self.cacheCheck),
          ( [self.widthSpin, self.heightSpin], SIGNAL( "valueChanged(int)" ), self.resizeGroupBox ),
          (self.multithreadCheck, SIGNAL("stateChanged(int)")),
          (self.noDataEdit, SIGNAL( "textChanged( const QString & )" ), self.noDataCheck), 
          (self.maskSelector, SIGNAL("filenameChanged()"), self.maskCheck, "1.6.0"), 
        ]
      )

      self.connect(self.inSelector, SIGNAL("layerChanged()"), self.fillSourceSRSEditDefault)
      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFile)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect(self.selectSourceSRSButton, SIGNAL("clicked()"), self.fillSourceSRSEdit)
      self.connect(self.selectTargetSRSButton, SIGNAL("clicked()"), self.fillTargetSRSEdit)
      self.connect(self.maskSelector, SIGNAL("selectClicked()"), self.fillMaskFile)
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )


  # switch to batch or normal mode
  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )
      self.progressBar.setVisible( self.batchCheck.isChecked() )

      self.inSelector.setType( self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER )
      self.outSelector.clear()

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.outFileLabel = self.label_2.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )
        self.label_2.setText( QCoreApplication.translate( "GdalTools", "&Output directory" ) )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.disconnect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFileEdit )

        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
        QObject.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputDir )
      else:
        self.label.setText( self.inFileLabel )
        self.label_2.setText( self.outFileLabel )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
        QObject.disconnect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputDir )

        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.connect( self.outSelector, SIGNAL( "selectClicked()" ), self.fillOutputFileEdit )

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )
      self.maskSelector.setLayers( filter( lambda x: x.geometryType() == QGis.Polygon, Utils.LayerRegistry.instance().getVectorLayers() ) )

  def fillInputFile(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Warp" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
      self.inSelector.setFilename(inputFile)

      # get SRS for source file if necessary and possible
      self.refreshSourceSRS()

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename(outputFile)

  def fillMaskFile(self):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      maskFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the mask file" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter )
      if maskFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)
      self.maskSelector.setFilename(maskFile)


  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files to Warp" ))
      if inputDir.isEmpty():
        return

      self.inSelector.setFilename( inputDir )

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
      self.outSelector.setFilename( outputDir )

  def fillSourceSRSEdit(self):
      dialog = SRSDialog( "Select the source SRS" )
      if dialog.exec_():
        self.sourceSRSEdit.setText(dialog.getProjection())

  def fillSourceSRSEditDefault(self):
      if self.inSelector.layer() == None:
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
      if self.maskCheck.isChecked():
        mask = self.getMaskFileName()
        if not mask.isEmpty():
          arguments << "-q"
          arguments << "-cutline"
          arguments << mask
          arguments << "-dstalpha"
      if self.isBatchEnabled():
        return arguments

      outputFn = self.getOutputFileName()
      if not outputFn.isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << outputFn
      return arguments

  def getInputFileName(self):
      return self.inSelector.filename()

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getMaskFileName(self):
      return self.maskSelector.filename()

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

