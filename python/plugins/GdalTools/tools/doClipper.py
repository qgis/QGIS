# -*- coding: utf-8 -*-

"""
***************************************************************************
    doClipper.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetClipper import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.canvas = self.iface.mapCanvas()

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_translate")

      self.outSelector.setType( self.outSelector.FILE )
      self.extentSelector.setCanvas(self.canvas)
      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()") ),
          (self.outSelector, SIGNAL("filenameChanged()") ),
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck, "1.7.0"),
          (self.maskSelector, SIGNAL("filenameChanged()"), self.maskModeRadio, "1.6.0"),
          (self.alphaBandCheck, SIGNAL( "stateChanged( int )") ),
          (self.extentSelector, [SIGNAL("selectionStarted()"), SIGNAL("newExtentDefined()")], self.extentModeRadio),
          (self.modeStackedWidget, SIGNAL("currentIndexChanged(int)"))
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect(self.maskSelector, SIGNAL("selectClicked()"), self.fillMaskFileEdit)
      self.connect(self.extentSelector, SIGNAL("newExtentDefined()"), self.checkRun)
      self.connect(self.extentSelector, SIGNAL("selectionStarted()"), self.checkRun)

      self.connect(self.extentModeRadio, SIGNAL("toggled(bool)"), self.switchClippingMode)

  def show_(self):
      self.switchClippingMode()
      BasePluginWidget.show_(self)

  def onClosing(self):
      self.extentSelector.stop()
      BasePluginWidget.onClosing(self)

  def switchClippingMode(self):
      if self.extentModeRadio.isChecked():
        index = 0
        self.extentSelector.start()
      else:
        self.extentSelector.stop()
        index = 1
      self.modeStackedWidget.setCurrentIndex( index )
      self.checkRun()

  def checkRun(self):
      if self.extentModeRadio.isChecked():
        enabler = self.extentSelector.isCoordsValid()
      else:
        enabler = not self.maskSelector.filename().isEmpty()
      self.base.enableRun( enabler )

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry.instance().getRasterLayers() )
      self.maskSelector.setLayers( filter( lambda x: x.geometryType() == QGis.Polygon, Utils.LayerRegistry.instance().getVectorLayers() ) )
      self.checkRun()

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Polygonize" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if inputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
      self.outSelector.setFilename(outputFile)

  def fillMaskFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
      maskFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the mask file" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter )
      if maskFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.maskSelector.setFilename(maskFile)
      self.checkRun()

  def getArguments(self):
      if not self.extentModeRadio.isChecked():
        return self.getArgsModeMask()
      return self.getArgsModeExtent()

  def getArgsModeExtent(self):
      self.base.setPluginCommand( "gdal_translate" )
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-a_nodata"
        arguments << str(self.noDataSpin.value())
      if self.extentModeRadio.isChecked() and self.extentSelector.isCoordsValid():
        rect = self.extentSelector.getExtent()
        if rect != None:
          arguments << "-projwin"
          arguments << str(rect.xMinimum())
          arguments << str(rect.yMaximum())
          arguments << str(rect.xMaximum())
          arguments << str(rect.yMinimum())
      if not self.getOutputFileName().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << self.getOutputFileName()
      return arguments

  def getArgsModeMask(self):
      self.base.setPluginCommand( "gdalwarp" )
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-dstnodata"
        arguments << str(self.noDataSpin.value())
      if self.maskModeRadio.isChecked():
        mask = self.maskSelector.filename()
        if not mask.isEmpty():
          arguments << "-q"
          arguments << "-cutline"
          arguments << mask
          if Utils.GdalConfig.version() >= "1.8.0":
            arguments << "-crop_to_cutline"
          if self.alphaBandCheck.isChecked():
            arguments << "-dstalpha"

      outputFn = self.getOutputFileName()
      if not outputFn.isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      arguments << self.getInputFileName()
      arguments << outputFn
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      return self.inSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

