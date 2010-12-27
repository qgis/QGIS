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
      BasePluginWidget.__init__(self, self.iface, "gdal_merge.py", None, self.iface.mainWindow())

      self.extentSelector.setCanvas(self.canvas)
      self.outputFormat = Utils.fillRasterOutputFormat()

      self.inputFiles = QStringList()
      self.warningDialog = QErrorMessage(self)

      self.setParamsStatus(
        [
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
          (self.pctCheck, SIGNAL("stateChanged(int)")), 
          ( self.extentSelector, [SIGNAL("selectionStarted()"), SIGNAL("newExtentDefined()")] )
        ]
      )

      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.extentSelector, SIGNAL("newExtentDefined()"), self.checkRun)
      self.connect(self.extentSelector, SIGNAL("selectionStarted()"), self.checkRun)

  def show_(self):
      self.connect(self.canvas, SIGNAL("layersChanged()"), self.fillInputFiles)
      self.extentSelector.start()
      BasePluginWidget.show_(self)

      self.fillInputFiles()
      self.checkRun()

  def onClosing(self):
      self.disconnect(self.canvas, SIGNAL("layersChanged()"), self.fillInputFiles)
      self.extentSelector.stop()
      BasePluginWidget.onClosing(self)

  def fillInputFiles(self):
      self.inputFiles = QStringList()

      # for each active layer
      for i in range(self.canvas.layerCount()): 
        layer = self.canvas.layer(i) 
        # only raster layers, but not WMS ones
        if layer.type() == layer.RasterLayer and ( not layer.usesProvider() ):
            # do not use the output file as input
            if layer.source() == self.outputFileEdit.text():
              continue

            self.inputFiles << layer.source()

      if self.inputFiles.isEmpty():
        self.extentSelector.stop()

        if self.isVisible() and self.warningDialog.isHidden():
          msg = QString( self.tr("No active raster layers. You must add almost one raster layer to continue.") )
          self.warningDialog.showMessage(msg)
      else:
        self.warningDialog.hide()
        self.extentSelector.start()

      self.checkRun()

  def checkRun(self):
      self.base.enableRun( not self.inputFiles.isEmpty() and self.extentSelector.getExtent() != None )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)

      # do not use the output file as input
      if self.inputFiles.contains(outputFile):
        self.inputFiles.removeAt( self.inputFiles.indexOf(outputFile) )
      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-n"
        arguments << str(self.noDataSpin.value())
      if self.pctCheck.isChecked():
        arguments << "-pct"
      if self.extentSelector.isCoordsValid():
        rect = self.extentSelector.getExtent()
        if rect != None:
          arguments << "-ul_lr"
          arguments << str(rect.xMinimum())
          arguments << str(rect.yMaximum())
          arguments << str(rect.xMaximum())
          arguments << str(rect.yMinimum())
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-o"
        arguments << self.outputFileEdit.text()
      arguments << self.inputFiles
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

