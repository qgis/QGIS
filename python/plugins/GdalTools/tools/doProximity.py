# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *

from osgeo import ogr

from ui_widgetProximity import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_proximity.py")

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.valuesEdit, SIGNAL("textChanged(const QString &)"), self.valuesCheck),
          (self.distUnitsCombo, SIGNAL("currentIndexChanged(int)"), self.distUnitsCheck),
          (self.maxDistSpin, SIGNAL("valueChanged(int)"), self.maxDistCheck),
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
          (self.fixedBufValSpin, SIGNAL("valueChanged(int)"), self.fixedBufValCheck)
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)

      # fill layers combo
      self.fillInputLayerCombo()

  def fillInputLayerCombo( self ):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Proximity" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << self.layers[ self.inputLayerCombo.currentIndex() ].source()
      else:
        arguments << self.inputLayerCombo.currentText()
      arguments << self.outputFileEdit.text()
      if self.valuesCheck.isChecked():
        values = self.valuesEdit.text().trimmed()
        if not values.isEmpty():
          arguments << "-values"
          arguments << values.replace(' ', ',')
      if self.distUnitsCheck.isChecked() and self.distUnitsCombo.currentIndex() >= 0:
        arguments << "-distunits"
        arguments << self.distUnitsCombo.currentText()
      if self.maxDistCheck.isChecked():
        arguments << "-maxdist"
        arguments << str(self.maxDistSpin.value())
      if self.noDataCheck.isChecked():
        arguments << "-nodata"
        arguments << str(self.noDataSpin.value())
      if self.fixedBufValCheck.isChecked():
        arguments << "-fixed-buf-val"
        arguments << str(self.fixedBufValSpin.value())
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())
