# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetClipper import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.canvas = self.iface.mapCanvas()

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_translate", None, self.iface.mainWindow())

      self.extentSelector.setCanvas(self.canvas)
      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
          ( self.extentSelector, [SIGNAL("selectionStarted()"), SIGNAL("newExtentDefined()")] )
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.extentSelector, SIGNAL("newExtentDefined()"), self.checkRun)
      self.connect(self.extentSelector, SIGNAL("selectionStarted()"), self.checkRun)

  def show_(self):
      self.extentSelector.start()
      BasePluginWidget.show_(self)

  def onClosing(self):
      self.extentSelector.stop()
      BasePluginWidget.onClosing(self)

  def checkRun(self):
      self.base.enableRun( self.extentSelector.getExtent() != None )

  def onLayersChanged(self):
      self.fillInputLayerCombo()

  def fillInputLayerCombo(self):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.LayerRegistry.instance().getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFileEdit( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the input file for Translate" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText( inputFile )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-a_nodata"
        arguments << str(self.noDataSpin.value())
      if self.extentSelector.isCoordsValid():
        rect = self.extentSelector.getExtent()
        if rect != None:
          arguments << "-projwin"
          arguments << str(rect.xMinimum())
          arguments << str(rect.yMaximum())
          arguments << str(rect.xMaximum())
          arguments << str(rect.yMinimum())
      if not self.getOutputFileName().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << self.getOutputFileName()
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def getInputFileName(self):
      if self.inputLayerCombo.currentIndex() >= 0:
        return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

