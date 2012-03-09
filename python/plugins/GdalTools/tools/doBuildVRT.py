# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from qgis.utils import *

from ui_widgetBuildVRT import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.resolutions = ("highest", "average", "lowest")

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdalbuildvrt")

      self.inSelector.setType( self.inSelector.FILE )
      self.outSelector.setType( self.outSelector.FILE )
      self.recurseCheck.hide()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")), 
          (self.outSelector, SIGNAL("filenameChanged()")), 
          (self.resolutionComboBox, SIGNAL("currentIndexChanged(int)"), self.resolutionCheck),
          (self.srcNoDataSpin, SIGNAL("valueChanged(int)"), self.srcNoDataCheck, "1.7.0"),
          (self.inputDirCheck, SIGNAL("stateChanged(int)")),
          (self.separateCheck, SIGNAL("stateChanged(int)"), None, "1.7.0"),
          (self.allowProjDiffCheck, SIGNAL("stateChanged(int)"), None, "1.7.0"),
          (self.recurseCheck, SIGNAL("stateChanged(int)"), self.inputDirCheck)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect( self.inputDirCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )
      self.connect( self.useSelectedLayersCheck, SIGNAL( "stateChanged( int )" ), self.switchLayerMode )

  def switchToolMode(self):
      self.recurseCheck.setVisible( self.inputDirCheck.isChecked() )
      self.inSelector.clear()

      if self.inputDirCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        QObject.disconnect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
        QObject.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputDir)
      else:
        self.label.setText( self.inFileLabel )

        QObject.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
        QObject.disconnect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputDir)

  def switchLayerMode(self):
      enableInputFiles = not self.useSelectedLayersCheck.isChecked()
      self.inputDirCheck.setEnabled( enableInputFiles )
      self.inSelector.setEnabled( enableInputFiles )
      self.recurseCheck.setEnabled( enableInputFiles )

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files for VRT" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
      self.inSelector.setFilename(files.join(","))

  def fillOutputFileEdit(self):
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the VRT" ), self.tr( "VRT (*.vrt)" ))
      if outputFile.isEmpty():
        return
      self.outSelector.setFilename(outputFile)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files for VRT" ))
      if inputDir.isEmpty():
        return
      self.inSelector.setFilename( inputDir )

  def getArguments(self):
      arguments = QStringList()
      if self.resolutionCheck.isChecked() and self.resolutionComboBox.currentIndex() >= 0:
        arguments << "-resolution"
        arguments << self.resolutions[self.resolutionComboBox.currentIndex()]
      if self.separateCheck.isChecked():
        arguments << "-separate"
      if self.srcNoDataCheck.isChecked():
        arguments << "-srcnodata"
        arguments << str(self.srcNoDataSpin.value())
      if self.allowProjDiffCheck.isChecked():
        arguments << "-allow_projection_difference"
      arguments << self.getOutputFileName()
      if self.useSelectedLayersCheck.isChecked():
        arguments << self.getInputFileNamesFromSelectedLayers()
      else:
        if self.inputDirCheck.isChecked():
          arguments << Utils.getRasterFiles( self.getInputFileName(), self.recurseCheck.isChecked() )
        else:
          arguments << self.getInputFileName()
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      if self.inputDirCheck.isChecked():
        return self.inSelector.filename()
      return self.inSelector.filename().split(",")

  def getInputFileNamesFromSelectedLayers(self):
      layers = iface.mapCanvas().layers()
      files = list()
      for layer in layers:
        if layer.type() == QgsMapLayer.RasterLayer and layer.providerType() == "gdal":
          files.append( str(layer.source()) )
      return files

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

