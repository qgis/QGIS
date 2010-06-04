# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

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

      self.setParamsStatus(
        [
          (self.inputFilesEdit, SIGNAL("textChanged(const QString &)")), 
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.resolutionComboBox, SIGNAL("currentIndexChanged(int)"), self.resolutionCheck),
          (self.srcNoDataSpin, SIGNAL("valueChanged(int)"), self.srcNoDataCheck, "1.7.0")
        ]
      )

      self.connect(self.selectInputFilesButton, SIGNAL("clicked()"), self.fillInputFilesEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files for VRT" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if files.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputFilesEdit.setText(files.join(","))

  def fillOutputFileEdit(self):
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the VRT" ), self.tr( "VRT (*.vrt)" ))
      if outputFile.isEmpty():
        return

      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.resolutionCheck.isChecked() and self.resolutionComboBox.currentIndex() >= 0:
        arguments << "-resolution"
        arguments << self.resolutions[self.resolutionComboBox.currentIndex()]
      if self.srcNoDataCheck.isChecked():
        arguments << "-srcnodata"
        arguments << str(self.srcNoDataSpin.value())
      arguments << self.outputFileEdit.text()
      arguments << self.inputFilesEdit.text().split(",")
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())
