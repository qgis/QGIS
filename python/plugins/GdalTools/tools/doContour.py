# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetContour import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_contour")

      gdalVersion = Utils.GdalConfig.version()
      self.useDirAsOutput = gdalVersion < "1.7.0"

      self.outSelector.setType( self.outSelector.FILE )

      # set the default QSpinBoxes value
      self.intervalDSpinBox.setValue(10.0)

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()") ),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.intervalDSpinBox, SIGNAL("valueChanged(double)")),
          (self.attributeEdit, SIGNAL("textChanged(const QString &)"), self.attributeCheck)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Contour" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      if not self.useDirAsOutput:
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        outputFile, encoding = Utils.FileDialog.getOpenFileName(self, self.tr( "Select where to save the Contour output" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      else:
        outputFile, encoding = Utils.FileDialog.getExistingDirectory(self, self.tr( "Select where to save the Contour output" ), True)

      if outputFile.isEmpty():
        return

      if not self.useDirAsOutput:
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.outSelector.setFilename(outputFile)
      self.lastEncoding = encoding

  def getArguments(self):
      arguments = QStringList()
      if self.attributeCheck.isChecked() and not self.attributeEdit.text().isEmpty():
        arguments << "-a"
        arguments << self.attributeEdit.text()
      if True: # XXX in this moment the -i argument is not optional
        arguments << "-i"
        arguments << QString(str(self.intervalDSpinBox.value()))
      arguments << self.getInputFileName()
      arguments << self.outSelector.filename()
      return arguments

  def getInputFileName(self):
      return self.inSelector.filename()

  def getOutputFileName(self):
      if self.useDirAsOutput:
        fn = self.outSelector.filename()
        return fn if fn.isEmpty() else fn + QDir.separator() + "contour.shp"
      return self.outSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      vl = self.iface.addVectorLayer(fileInfo.filePath(), fileInfo.baseName(), "ogr")
      if vl != None and vl.isValid():
        if hasattr(self, 'lastEncoding'):
          vl.setProviderEncoding(self.lastEncoding)
