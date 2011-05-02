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

      self.outSelector.setType( self.outSelector.FILE )
      self.outputFormat = Utils.fillVectorOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.fieldEdit, SIGNAL("textChanged(const QString &)"), self.fieldCheck)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Polygonize" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      outputFile, encoding = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the Polygonize output" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.outputFormat = Utils.fillVectorOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename(outputFile)
      self.lastEncoding = encoding

  def getArguments(self):
      arguments = QStringList()
      arguments << self.getInputFileName()
      outputFn = self.getOutputFileName()
      if not outputFn.isEmpty():
        arguments << "-f"
        arguments << self.outputFormat
      arguments << outputFn
      if not outputFn.isEmpty():
        arguments << QFileInfo( outputFn ).baseName()
      if self.fieldCheck.isChecked() and not self.fieldEdit.text().isEmpty():
        arguments << self.fieldEdit.text()
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      return self.inSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      vl = self.iface.addVectorLayer(fileInfo.filePath(), fileInfo.baseName(), "ogr")
      if vl != None and vl.isValid():
        if hasattr(self, 'lastEncoding'):
          vl.setProviderEncoding(self.lastEncoding)

