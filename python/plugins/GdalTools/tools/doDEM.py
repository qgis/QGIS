# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetDEM import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.modes = ("hillshade", "slope", "aspect", "color-relief", "TRI", "TPI", "roughness")

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdaldem")

      # set the default QSpinBoxes and QProgressBar value
      self.bandSpin.setValue(1)

      self.hillshadeZFactorSpin.setValue(1)
      self.hillshadeScaleSpin.setValue(1)
      self.hillshadeAltitudeSpin.setValue(45.0)
      self.hillshadeAzimuthSpin.setValue(315.0)
      self.slopeScaleSpin.setValue(1)

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ), 
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.computeEdgesCheck, SIGNAL("stateChanged(int)"), None, "1.8.0"), 
          (self.bandSpin, SIGNAL("valueChanged(int)"), self.bandCheck), 
          (self.algorithmCheck, SIGNAL("stateChanged(int)"), None, "1.8.0"), 
          (self.creationOptionsTable, [SIGNAL("cellValueChanged(int, int)"), SIGNAL("rowRemoved()")], self.creationGroupBox), 
          (self.modeCombo, SIGNAL("currentIndexChanged(int)")),
          ([self.hillshadeZFactorSpin, self.hillshadeScaleSpin, self.hillshadeAltitudeSpin, self.hillshadeAzimuthSpin], SIGNAL("valueChanged(double)")), 
          (self.slopeScaleSpin, SIGNAL("valueChanged(double)")), 
          (self.slopePercentCheck, SIGNAL("stateChanged(int)")), 
          ([self.aspectTrigonometricCheck, self.aspectZeroForFlatCheck], SIGNAL("stateChanged(int)")), 
          (self.colorConfigFileEdit, SIGNAL("textChanged(const QString &)")), 
          ([self.colorExactRadio, self.colorNearestRadio], SIGNAL("toggled(bool)"), self.colorMatchGroupBox), 
          (self.colorAlphaCheck, SIGNAL("stateChanged(int)"))
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.colorSelectConfigFileButton, SIGNAL("clicked()"), self.fillColorConfigFileEdit)
      self.connect(self.modeCombo, SIGNAL("currentIndexChanged(int)"), self.showModeParams)

  def showModeParams(self, index):
      self.stackedWidget.setVisible( index < 4 )
      self.algorithmCheck.setVisible( index < 3 )
      if index >= 3:
        self.algorithmCheck.setChecked( False )

  def onLayersChanged(self):
      self.fillInputLayerCombo()

  def fillInputLayerCombo(self):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.LayerRegistry.instance().getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the file for DEM" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
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

  def fillColorConfigFileEdit(self):
      configFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the color configuration file" ), "*")
      if configFile.isEmpty():
        return

      self.colorConfigFileEdit.setText(configFile)

  def getArguments(self):
      mode = self.modes[ self.modeCombo.currentIndex() ]
      arguments = QStringList()
      arguments << mode
      arguments << self.getInputFileName()
      if mode == "color-relief":
        arguments << self.colorConfigFileEdit.text()
      arguments << self.outputFileEdit.text()
      if mode == "hillshade":
        arguments << "-z" << str(self.hillshadeZFactorSpin.value())
        arguments << "-s" << str(self.hillshadeScaleSpin.value())
        arguments << "-az" << str(self.hillshadeAzimuthSpin.value())
        arguments << "-alt" << str(self.hillshadeAltitudeSpin.value())
      elif mode == "slope":
        if self.slopePercentCheck.isChecked():
          arguments << "-p"
        arguments << "-s" << str(self.slopeScaleSpin.value())
      elif mode == "aspect":
        if self.aspectTrigonometricCheck.isChecked():
          arguments << "-trigonometric"
        if self.aspectZeroForFlatCheck.isChecked():
          arguments << "-zero_for_flat"
      elif mode == "color-relief":
        if self.colorAlphaCheck.isChecked():
          arguments << "-alpha"
        if self.colorMatchGroupBox.isChecked():
          if self.colorExactRadio.isChecked():
            arguments << "-exact_color_entry"
          elif self.colorNearestRadio.isChecked():
            arguments << "-nearest_color_entry"
      if self.algorithmCheck.isChecked():
        arguments << "-alg" << "ZevenbergenThorne"
      if self.computeEdgesCheck.isChecked():
        arguments << "-compute_edges"
      if self.bandCheck.isChecked():
        arguments << "-b" << str(self.bandSpin.value())
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of" << self.outputFormat
      if self.creationGroupBox.isChecked():
        for opt in self.creationOptionsTable.options():
          arguments << "-co" << opt
      return arguments

  def getInputFileName(self):
      if self.inputLayerCombo.currentIndex() >= 0:
        return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

