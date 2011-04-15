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
      self.layers = []
      self.maskLayers = []

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck, "1.7.0"),
          (self.maskLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")], self.maskModeRadio, "1.6.0"),
          ( self.extentSelector, [SIGNAL("selectionStarted()"), SIGNAL("newExtentDefined()")] ), 
          (self.modeStackedWidget, SIGNAL("currentIndexChanged(int)"))
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.selectMaskFileButton, SIGNAL("clicked()"), self.fillMaskFileEdit)
      self.connect(self.extentSelector, SIGNAL("newExtentDefined()"), self.checkRun)
      self.connect(self.extentSelector, SIGNAL("selectionStarted()"), self.checkRun)

      self.connect(self.extentModeRadio, SIGNAL("toggled(bool)"), self.switchClippingMode)

  def show_(self):
      self.switchClippingMode()
      BasePluginWidget.show_(self)

  def onClosing(self):
      self.extentSelector.stop()
      BasePluginWidget.onClosing(self)

  def switchClippingMode(self):
      if self.extentModeRadio.isChecked():
        index = 0
        self.extentSelector.start()
      else:
        self.extentSelector.stop()
        index = 1
      self.modeStackedWidget.setCurrentIndex( index )
      self.checkRun()

  def checkRun(self):
      if self.extentModeRadio.isChecked():
        enabler = self.extentSelector.isCoordsValid()
      else:
        enabler = not self.getMaskFileName().isEmpty()
      self.base.enableRun( enabler )

  def onLayersChanged(self):
      self.fillInputLayerCombo()
      self.fillMaskLayerCombo()

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

  def fillMaskLayerCombo(self):
      self.maskLayerCombo.clear()
      self.maskLayers = filter( lambda x: x.geometryType() == QGis.Polygon, Utils.LayerRegistry.instance().getVectorLayers()[0] )
      self.maskLayerCombo.addItems( map( lambda x: x.name(), self.maskLayers ) )
      self.checkRun()

  def fillMaskFileEdit( self ):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      maskFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the mask file" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter )
      if maskFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.maskLayerCombo.setCurrentIndex(-1)
      self.maskLayerCombo.setEditText( maskFile )
      self.checkRun()

  def getArguments(self):
      if not self.extentModeRadio.isChecked():
        return self.getArgsModeMask()
      return self.getArgsModeExtent()

  def getArgsModeExtent(self):
      self.base.setPluginCommand( "gdal_translate" )
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-a_nodata"
        arguments << str(self.noDataSpin.value())
      if self.extentModeRadio.isChecked() and self.extentSelector.isCoordsValid():
        rect = self.extentSelector.getExtent()
        if rect != None:
          arguments << "-projwin"
          arguments << str(rect.xMinimum())
          arguments << str(rect.yMaximum())
          arguments << str(rect.xMaximum())
          arguments << str(rect.yMinimum())
      if Utils.GdalConfig.version() >= "1.7.0":
        arguments << "-q"
      if not self.getOutputFileName().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << self.getOutputFileName()
      return arguments

  def getArgsModeMask(self):
      self.base.setPluginCommand( "gdalwarp" )
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-dstnodata"
        arguments << str(self.noDataSpin.value())
      if self.maskModeRadio.isChecked():
        mask = self.getMaskFileName()
        if not mask.isEmpty():
          arguments << "-q"
          arguments << "-cutline"
          arguments << mask
          arguments << "-dstalpha"

      outputFn = self.getOutputFileName()
      if not outputFn.isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << outputFn
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def getInputFileName(self):
      if self.inputLayerCombo.currentIndex() >= 0:
        return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def getMaskFileName(self):
      if self.maskLayerCombo.currentIndex() >= 0:
        return self.maskLayers[self.maskLayerCombo.currentIndex()].source()
      return self.maskLayerCombo.currentText()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

