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

      # we use one widget for two tools, so change control labels
      self.base.setWindowTitle( self.tr( "Convert paletted image to RGB" ) )
      self.colorsCheck.setText( self.tr( "Band to convert:" ) )
      self.colorsSpin.setRange( 1, 256 )
      self.colorsSpin.setValue( 1 )

      self.progressBar.hide()

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [ SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)") ] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.colorsSpin, SIGNAL("valueChanged(int)"), self.colorsCheck),
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFile)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

      self.fillInputLayerCombo()

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

  def fillInputLayerCombo(self):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFile(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for convert" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)

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

  def fillOutputDir( self ):
      outputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the output directory to save the results to" ))
      if outputDir.isEmpty():
        return

      self.outputFileEdit.setText( outputDir )

  def getArguments(self):
      arguments = QStringList()
      if self.colorsCheck.isChecked():
        arguments << "-b"
        arguments << str( self.colorsSpin.value() )
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
