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

      self.outSelector.setType( self.outSelector.FILE )
      self.configSelector.setType( self.configSelector.FILE )

      # set the default QSpinBoxes and QProgressBar value
      self.bandSpin.setValue(1)

      self.hillshadeZFactorSpin.setValue(1)
      self.hillshadeScaleSpin.setValue(1)
      self.hillshadeAltitudeSpin.setValue(45.0)
      self.hillshadeAzimuthSpin.setValue(315.0)
      self.slopeScaleSpin.setValue(1)

      # set the default color configuration file to terrain
      import os.path
      colorConfigFile = os.path.join(os.path.dirname(__file__), "terrain.txt")
      self.configSelector.setFilename(colorConfigFile)

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")), 
          (self.outSelector, SIGNAL("filenameChanged()")), 
          (self.computeEdgesCheck, SIGNAL("stateChanged(int)"), None, "1.8.0"), 
          (self.bandSpin, SIGNAL("valueChanged(int)"), self.bandCheck), 
          (self.algorithmCheck, SIGNAL("stateChanged(int)"), None, "1.8.0"), 
          (self.creationOptionsTable, [SIGNAL("cellValueChanged(int, int)"), SIGNAL("rowRemoved()")], self.creationGroupBox), 
          (self.modeCombo, SIGNAL("currentIndexChanged(int)")),
          ([self.hillshadeZFactorSpin, self.hillshadeScaleSpin, self.hillshadeAltitudeSpin, self.hillshadeAzimuthSpin], SIGNAL("valueChanged(double)")), 
          (self.slopeScaleSpin, SIGNAL("valueChanged(double)")), 
          (self.slopePercentCheck, SIGNAL("stateChanged(int)")), 
          ([self.aspectTrigonometricCheck, self.aspectZeroForFlatCheck], SIGNAL("stateChanged(int)")), 
          (self.configSelector, SIGNAL("filenameChanged()")), 
          ([self.colorExactRadio, self.colorNearestRadio], SIGNAL("toggled(bool)"), self.colorMatchGroupBox), 
          (self.colorAlphaCheck, SIGNAL("stateChanged(int)"))
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect(self.configSelector, SIGNAL("selectClicked()"), self.fillColorConfigFileEdit)
      self.connect(self.modeCombo, SIGNAL("currentIndexChanged(int)"), self.showModeParams)

  def showModeParams(self, index):
      self.stackedWidget.setVisible( index < 4 )
      self.algorithmCheck.setVisible( index < 3 )
      if index >= 3:
        self.algorithmCheck.setChecked( False )

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the file for DEM" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
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

  def fillColorConfigFileEdit(self):
      configFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the color configuration file" ))
      if configFile.isEmpty():
        return
      self.configSelector.setFilename(configFile)

  def getArguments(self):
      mode = self.modes[ self.modeCombo.currentIndex() ]
      arguments = QStringList()
      arguments << mode
      arguments << self.getInputFileName()
      if mode == "color-relief":
        arguments << self.configSelector.filename()
      outputFn = self.getOutputFileName()
      arguments << outputFn
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
      if not outputFn.isEmpty():
        arguments << "-of" << self.outputFormat
      if self.creationGroupBox.isChecked():
        for opt in self.creationOptionsTable.options():
          arguments << "-co" << opt
      return arguments

  def getInputFileName(self):
      return self.inSelector.filename()

  def getOutputFileName(self):
      return self.outSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

