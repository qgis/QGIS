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


class GdalToolsDialog( QWidget, Ui_Widget, BaseBatchWidget ):

  def __init__( self, iface ):
      QWidget.__init__( self )
      self.iface = iface

      self.setupUi( self )
      BaseBatchWidget.__init__( self, self.iface, "gdalwarp" )

      # set the default QSpinBoxes and QProgressBar value
      self.progressBar.setValue(0)

      self.progressBar.hide()
      self.recurseCheck.hide()

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
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        self.progressBar.show()
        self.recurseCheck.show()

        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )
        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )

        #QObject.disconnect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.checkLayer )
      else:
        self.label.setText( self.inFileLabel )

        self.progressBar.hide()
        self.recurseCheck.hide()

        QObject.connect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputFileEdit )
        QObject.disconnect( self.selectInputFileButton, SIGNAL( "clicked()" ), self.fillInputDir )

        #QObject.connect( self.base.buttonBox.button( QDialogButtonBox.Ok ), SIGNAL( "clicked()" ), self.checkLayer )

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
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the file to analyse" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )
      #self.outputFormat = Utils.fillOutputFormat( lastUsedFilter, file )
      self.inputFileEdit.setText( inputFile )

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files to Assign projection" ))
      if inputDir.isEmpty():
        return

      self.inputFileEdit.setText( inputDir )

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
      self.tempFile = self.inputFileEdit.text().replace( QRegExp( "\.[a-zA-Z]{2,4}$" ), ".tif" ).append( ".tmp" )
      arguments << self.tempFile
      return arguments

  def getInputFileName(self):
      return self.inputFileEdit.text()

  def getOutputFileName( self ):
      return self.inputFileEdit.text()

  def getBatchOutputFileName(self, fn):
      # get GeoTiff
      fn = QString( fn ).replace( QRegExp( "\.[a-zA-Z]{2,4}$" ), ".tif" )
      return BaseBatchWidget.getBatchOutputFileName( self, fn )

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def checkLayer( self ):
      layerList = []

      layerMap = QgsMapLayerRegistry.instance().mapLayers()
      for name, layer in layerMap.iteritems():
        if layer.type() == QgsMapLayer.RasterLayer:
          layerList.append( unicode( layer.source() ) )

      if unicode( self.inputFileEdit.text() ) in layerList:
        QMessageBox.warning( self, self.tr( "Assign projection" ), self.tr( "This raster already found in map canvas" ) )
        return

      self.onRun()

  def isBatchEnabled(self):
      return self.batchCheck.isChecked()

  def isRecursiveScanEnabled(self):
      return self.recurseCheck.isChecked()

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

