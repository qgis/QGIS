# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetPolygonize import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.resolutions = ("highest", "average", "lowest")

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_polygonize.py")

      self.outputFormat = Utils.fillVectorOutputFormat()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.fieldEdit, SIGNAL("textChanged(const QString &)"), self.fieldCheck)
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)

  def onLayersChanged(self):
      self.fillInputLayerCombo()

  def fillInputLayerCombo( self ):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.LayerRegistry.instance().getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Polygonize" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      outputFile, encoding = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the Polygonize output" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.outputFormat = Utils.fillVectorOutputFormat( lastUsedFilter, outputFile )
      self.outputFileEdit.setText(outputFile)
      self.lastEncoding = encoding

  def getArguments(self):
      arguments = QStringList()
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << self.layers[ self.inputLayerCombo.currentIndex() ].source()
      else:
        arguments << self.inputLayerCombo.currentText()
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-f"
        arguments << self.outputFormat
      arguments << self.outputFileEdit.text()
      if not self.outputFileEdit.text().isEmpty():
        arguments << QFileInfo(self.outputFileEdit.text()).baseName()
      if self.fieldCheck.isChecked() and not self.fieldEdit.text().isEmpty():
        arguments << self.fieldEdit.text()
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      vl = self.iface.addVectorLayer(fileInfo.filePath(), fileInfo.baseName(), "ogr")
      if vl != None and vl.isValid():
        if hasattr(self, 'lastEncoding'):
          vl.setProviderEncoding(self.lastEncoding)
