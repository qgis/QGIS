# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetOverview import Ui_GdalToolsWidget as Ui_Widget
from widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
import GdalTools_utils as Utils

import platform

class GdalToolsDialog( QWidget, Ui_Widget, BaseBatchWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface
      self.resampling_method = ('nearest', 'average', 'gauss', 'average_mp', 'average_magphase', 'mode')

      self.setupUi( self )
      BaseBatchWidget.__init__( self, self.iface, "gdaladdo" )

      # set the default QSpinBoxes and QProgressBar value
      self.progressBar.setValue(0)
      self.jpegQualitySpin.setValue(80)

      self.progressBar.hide()
      # we don't need load to canvas functionality
      self.base.loadCheckBox.hide()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          ( self.algorithmCombo, SIGNAL( "currentIndexChanged( int )" ), self.algorithmCheck ),
          ( self.levelsEdit, SIGNAL( "textChanged( const QString & )" ) ),
          ( self.roModeCheck, SIGNAL( "stateChanged( int )" ), None, "1.6.0" ),
          ( self.rrdCheck, SIGNAL( "stateChanged(int)" ) ),
          ( self.jpegQualitySpin, SIGNAL( "valueChanged (int)" ) ),
          ( self.jpegQualityContainer, None, self.tiffjpegCheck),
          ( self.jpegQualityContainer, None, None, "1.7.0"),
          ( self.cleanCheck, SIGNAL( "stateChanged(int)" ), None, "1.7.0" )
        ]
      )

      self.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
      self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )


  # switch to batch or normal mode
  def switchToolMode( self ):
      self.setCommandViewerEnabled( not self.batchCheck.isChecked() )
      self.progressBar.setVisible( self.batchCheck.isChecked() )

      self.inSelector.setType( self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER )

      if self.batchCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )
        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
      else:
        self.label.setText( self.inFileLabel )

        QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
        QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFile )

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def fillInputFile( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the input file" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      self.inSelector.setFilename( inputFile )

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files" ))
      if inputDir.isEmpty():
        return

      self.inSelector.setFilename( inputDir )

  def getArguments( self ):
      arguments = QStringList()
      if self.algorithmCheck.isChecked() and self.algorithmCombo.currentIndex() >= 0:
        arguments << "-r"
        arguments << self.resampling_method[self.algorithmCombo.currentIndex()]
      if self.roModeCheck.isChecked():
        arguments << "-ro"
      if self.rrdCheck.isChecked():
        arguments << "--config" << "USE_RRD" << "YES"
      if self.tiffjpegCheck.isChecked():
        arguments << "--config" << "COMPRESS_OVERVIEW" << "JPEG" << "--config" << "PHOTOMETRIC_OVERVIEW" << "YCBCR" << "--config" << "INTERLEAVE_OVERVIEW" << "PIXEL"
        if self.jpegQualityContainer.isVisible():
            arguments << "--config" << "JPEG_QUALITY_OVERVIEW" << self.jpegQualitySpin.cleanText()
      if self.cleanCheck.isChecked():
          arguments << "-clean"
      if self.isBatchEnabled():
        return arguments

      arguments << self.getInputFileName()
      if not self.levelsEdit.text().isEmpty():
        arguments << self.levelsEdit.text().split( " " )
      else:
        arguments << "2" << "4" << "8" << "16" << "32"
      return arguments

  def getInputFileName( self ):
      return self.inSelector.filename()

  def getOutputFileName( self ):
      return self.inSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def getBatchArguments(self, inFile, outFile = None):
      arguments = self.getArguments()
      arguments << inFile
      if not self.levelsEdit.text().isEmpty():
        arguments << self.levelsEdit.text().split( " " )
      else:
        arguments << "2" << "4" << "8" << "16" << "32"
      return arguments

  def isBatchEnabled(self):
      return self.batchCheck.isChecked()

  def onFinished(self, exitCode, status):
      if not self.isBatchEnabled():
        from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
        BasePluginWidget.onFinished(self, exitCode, status)
        return

      msg = QString( self.base.process.readAllStandardError() )
      if not msg.isEmpty():
        self.errors.append( ">> " + self.inFiles[self.batchIndex] + "<br>" + msg.replace( "\n", "<br>" ) )

      self.base.process.close()
      self.batchIndex += 1
      self.runItem( self.batchIndex, self.batchTotal )

  def setProgressRange(self, maximum):
      self.progressBar.setRange(0, maximum)

  def updateProgress(self, index, total):
      if index < total:
        self.progressBar.setValue( index + 1 )
      else:
        self.progressBar.setValue( 0 )

