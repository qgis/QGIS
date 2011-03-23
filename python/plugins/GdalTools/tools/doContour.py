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

      # set the default QSpinBoxes value
      self.intervalDSpinBox.setValue(10.0)

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputDirEdit, SIGNAL("textChanged(const QString &)")),
          (self.intervalDSpinBox, SIGNAL("valueChanged(double)")),
          (self.attributeEdit, SIGNAL("textChanged(const QString &)"), self.attributeCheck)
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputDirButton, SIGNAL("clicked()"), self.fillOutputDirEdit)

  def onLayersChanged(self):
      self.fillInputLayerCombo()

  def fillInputLayerCombo( self ):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.LayerRegistry.instance().getRasterLayers()
      self.inputLayerCombo.addItems( names )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Contour" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)

  def fillOutputDirEdit(self):
      outputDir, encoding = Utils.FileDialog.getExistingDirectory(self, self.tr( "Select where to save the Contour output" ), True)
      if outputDir.isEmpty():
        return

      self.outputDirEdit.setText(outputDir)
      self.lastEncoding = encoding

  def getArguments(self):
      arguments = QStringList()
      if self.attributeCheck.isChecked() and not self.attributeEdit.text().isEmpty():
        arguments << "-a"
        arguments << self.attributeEdit.text()
      if True: # XXX in this moment the -i argument is not optional
        arguments << "-i"
        arguments << QString(str(self.intervalDSpinBox.value()))
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << self.layers[ self.inputLayerCombo.currentIndex() ].source()
      else:
        arguments << self.inputLayerCombo.currentText()
      arguments << self.outputDirEdit.text()
      return arguments

  def getOutputFileName(self):
      if self.outputDirEdit.text().isEmpty():
        return ""
      return self.outputDirEdit.text() + QDir.separator() + "contour.shp"

  def addLayerIntoCanvas(self, fileInfo):
      vl = self.iface.addVectorLayer(fileInfo.filePath(), "contour", "ogr")
      if vl != None and vl.isValid():
        if hasattr(self, 'lastEncoding'):
          vl.setProviderEncoding(self.lastEncoding)
