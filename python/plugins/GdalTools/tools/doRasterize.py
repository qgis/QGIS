# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from osgeo import ogr

from ui_widgetRasterize import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_rasterize")

      self.lastEncoding = Utils.getLastUsedEncoding()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.attributeComboBox, SIGNAL("currentIndexChanged(int)"))
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.inputLayerCombo, SIGNAL("currentIndexChanged(int)"), self.fillFieldsCombo)

      # fill layers combo
      self.fillInputLayerCombo()

  def fillInputLayerCombo( self ):
      self.inputLayerCombo.clear()
      ( self.layers, names ) = Utils.getVectorLayers()
      self.inputLayerCombo.addItems( names )

  def fillFieldsCombo(self):
      index = self.inputLayerCombo.currentIndex()
      if index < 0:
        return

      self.lastEncoding = self.layers[index].dataProvider().encoding()
      self.loadFields( self.getInputFileName() )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      inputFile, encoding = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Rasterize" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)
      self.lastEncoding = encoding

      self.loadFields( inputFile )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.attributeComboBox.currentIndex() >= 0:
        arguments << "-a"
        arguments << self.attributeComboBox.currentText()
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << "-l"
        arguments << QFileInfo(self.layers[ self.inputLayerCombo.currentIndex() ].source()).baseName()
      elif not self.inputLayerCombo.currentText().isEmpty():
        arguments << "-l"
        arguments << QFileInfo(self.inputLayerCombo.currentText()).baseName()
      arguments << self.getInputFileName()
      arguments << self.outputFileEdit.text()
      return arguments

  def getInputFileName(self):
      if self.inputLayerCombo.currentIndex() >= 0:
        return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def loadFields(self, vectorFile):
      self.attributeComboBox.clear()

      if vectorFile.isEmpty():
        return

      try:
        (fields, names) = Utils.getVectorFields(vectorFile)
      except Utils.UnsupportedOGRFormat, e:
        QErrorMessage(self).showMessage( e.args[0] )

        self.inputLayerCombo.clearEditText()
        self.inputLayerCombo.setCurrentIndex(-1)
        return

      ncodec = QTextCodec.codecForName(self.lastEncoding)
      for name in names:
        self.attributeComboBox.addItem( ncodec.toUnicode(name) )
