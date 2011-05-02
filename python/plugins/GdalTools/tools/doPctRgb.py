# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetConvert import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BaseBatchWidget.__init__(self, self.iface, "pct2rgb.py")

      # we use one widget for two tools
      self.base.setWindowTitle( self.tr( "Convert paletted image to RGB" ) )

      self.outSelector.setType( self.outSelector.FILE )

      # set the default QSpinBoxes and QProgressBar value
      self.bandSpin.setValue(1)
      self.progressBar.setValue(0)

      self.progressBar.hide()
      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.colorsSpin, SIGNAL("valueChanged(int)"), self.colorsCheck, "-1"),   # hide this option
          (self.bandSpin, SIGNAL("valueChanged(int)"), self.bandCheck)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFile)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
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

  def fillInputFile(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for convert" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename(outputFile)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files for convert" ))
      if inputDir.isEmpty():
        return
      self.inSelector.setFilename( inputDir )

  def fillOutputDir( self ):
      outputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the output directory to save the results to" ))
      if outputDir.isEmpty():
        return
      self.outSelector.setFilename( outputDir )

  def getArguments(self):
      arguments = QStringList()
      if self.bandCheck.isChecked():
        arguments << "-b"
        arguments << str( self.bandSpin.value() )
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

