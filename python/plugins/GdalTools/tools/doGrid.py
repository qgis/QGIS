# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetGrid import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.canvas = self.iface.mapCanvas()
      self.algorithm = ('invdist', 'average', 'nearest', 'datametrics')
      self.datametrics = ('minimum', 'maximum', 'range')

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_grid")

      self.extentSelector.setCanvas(self.canvas)
      #self.extentSelector.stop()

      # set the default QSpinBoxes value
      self.invdistPowerSpin.setValue(2.0)

      self.outputFormat = Utils.fillRasterOutputFormat()
      self.lastEncoding = Utils.getLastUsedEncoding()

      self.setParamsStatus(
        [
          (self.inputLayerCombo, [SIGNAL("currentIndexChanged(int)"), SIGNAL("editTextChanged(const QString &)")] ),
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")),
          (self.zfieldCombo, SIGNAL("currentIndexChanged(int)"), self.zfieldCheck),
          (self.algorithmCombo, SIGNAL("currentIndexChanged(int)"), self.algorithmCheck),
          (self.stackedWidget, SIGNAL("currentChanged(int)"), self.algorithmCheck),
          ([self.invdistPowerSpin, self.invdistSmothingSpin, self.invdistRadius1Spin, self.invdistRadius2Spin, self.invdistAngleSpin, self.invdistNoDataSpin], SIGNAL("valueChanged(double)")),
          ([self.invdistMaxPointsSpin, self.invdistMinPointsSpin], SIGNAL("valueChanged(int)")),
          ([self.averageRadius1Spin, self.averageRadius2Spin, self.averageAngleSpin, self.averageNoDataSpin], SIGNAL("valueChanged(double)")),
          (self.averageMinPointsSpin, SIGNAL("valueChanged(int)")),
          ([self.nearestRadius1Spin, self.nearestRadius2Spin, self.nearestAngleSpin, self.nearestNoDataSpin], SIGNAL("valueChanged(double)")),
          (self.datametricsCombo, SIGNAL("currentIndexChanged(int)")),
          ([self.datametricsRadius1Spin, self.datametricsRadius2Spin, self.datametricsAngleSpin, self.datametricsNoDataSpin], SIGNAL("valueChanged(double)")),
          (self.datametricsMinPointsSpin, SIGNAL("valueChanged(int)")), 
          (self.extentSelector, [SIGNAL("selectionStarted()"), SIGNAL("newExtentDefined()")], self.extentGroup)
        ]
      )

      self.connect(self.selectInputFileButton, SIGNAL("clicked()"), self.fillInputFileEdit)
      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.inputLayerCombo, SIGNAL("currentIndexChanged(int)"), self.fillFieldsCombo)
      self.connect(self.extentGroup, SIGNAL("toggled(bool)"), self.onExtentCheckedChenged)

      # fill layers combo
      self.fillInputLayerCombo()

  def onClosing(self):
      self.extentSelector.stop()
      BasePluginWidget.onClosing(self)

  def onExtentCheckedChenged(self, enabled):
        self.extentSelector.start() if enabled else self.extentSelector.stop()

  def fillInputLayerCombo(self):
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
      inputFile, encoding = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Grid" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.inputLayerCombo.setCurrentIndex(-1)
      self.inputLayerCombo.setEditText(inputFile)
      self.lastEncoding = encoding

      self.loadFields( inputFile )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.zfieldCheck.isChecked() and self.zfieldCombo.currentIndex() >= 0:
        arguments << "-zfield"
        arguments << self.zfieldCombo.currentText()
      if self.inputLayerCombo.currentIndex() >= 0:
        arguments << "-l"
        arguments << QFileInfo(self.layers[ self.inputLayerCombo.currentIndex() ].source()).baseName()
      elif not self.inputLayerCombo.currentText().isEmpty():
        arguments << "-l"
        arguments << QFileInfo(self.inputLayerCombo.currentText()).baseName()
      if self.extentGroup.isChecked():
        rect = self.extentSelector.getExtent()
        if rect != None:
          arguments << "-txe"
          arguments << str(rect.xMinimum())
          arguments << str(rect.xMaximum())
          arguments << "-tye"
          arguments << str(rect.yMaximum())
          arguments << str(rect.yMinimum())
      if self.algorithmCheck.isChecked() and self.algorithmCombo.currentIndex() >= 0:
        arguments << "-a"
        arguments << self.algorithmArguments(self.algorithmCombo.currentIndex())
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << self.outputFileEdit.text()
      return arguments

  def getInputFileName(self):
      #if self.inputLayerCombo.currentIndex() >= 0:
      #  return self.layers[self.inputLayerCombo.currentIndex()].source()
      return self.inputLayerCombo.currentText()

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

  def algorithmArguments(self, index):
      algorithm = self.algorithm[index]
      arguments = QStringList()
      if algorithm == "invdist":
        arguments.append(algorithm)
        arguments.append("power=" + str(self.invdistPowerSpin.value()))
        arguments.append("smothing=" + str(self.invdistSmothingSpin.value()))
        arguments.append("radius1=" + str(self.invdistRadius1Spin.value()))
        arguments.append("radius2=" + str(self.invdistRadius2Spin.value()))
        arguments.append("angle=" + str(self.invdistAngleSpin.value()))
        arguments.append("max_points=" + str(self.invdistMaxPointsSpin.value()))
        arguments.append("min_points=" + str(self.invdistMinPointsSpin.value()))
        arguments.append("nodata=" + str(self.invdistNoDataSpin.value()))
      elif algorithm == "average":
        arguments.append(algorithm)
        arguments.append("radius1=" + str(self.averageRadius1Spin.value()))
        arguments.append("radius2=" + str(self.averageRadius2Spin.value()))
        arguments.append("angle=" + str(self.averageAngleSpin.value()))
        arguments.append("min_points=" + str(self.averageMinPointsSpin.value()))
        arguments.append("nodata=" + str(self.averageNoDataSpin.value()))
      elif algorithm == "nearest":
        arguments.append(algorithm)
        arguments.append("radius1=" + str(self.nearestRadius1Spin.value()))
        arguments.append("radius2=" + str(self.nearestRadius2Spin.value()))
        arguments.append("angle=" + str(self.nearestAngleSpin.value()))
        arguments.append("nodata=" + str(self.nearestNoDataSpin.value()))
      else:
        arguments.append(self.datametrics[self.datametricsCombo.currentIndex()])
        arguments.append("radius1=" + str(self.datametricsRadius1Spin.value()))
        arguments.append("radius2=" + str(self.datametricsRadius2Spin.value()))
        arguments.append("angle=" + str(self.datametricsAngleSpin.value()))
        arguments.append("min_points=" + str(self.datametricsMinPointsSpin.value()))
        arguments.append("nodata=" + str(self.datametricsNoDataSpin.value()))
      return arguments.join(":")

  def loadFields(self, vectorFile = QString()):
      self.zfieldCombo.clear()

      if vectorFile.isEmpty():
        return

      try:
        (fields, names) = Utils.getVectorFields(vectorFile)
      except Exception, e:
        QErrorMessage(self).showMessage( str(e) )

        self.inputLayerCombo.clearEditText()
        self.inputLayerCombo.setCurrentIndex(-1)
        return

      ncodec = QTextCodec.codecForName(self.lastEncoding)
      for name in names:
        self.zfieldCombo.addItem( ncodec.toUnicode(name) )
