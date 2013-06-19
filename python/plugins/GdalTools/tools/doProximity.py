# -*- coding: utf-8 -*-

"""
***************************************************************************
    doProximity.py
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

from osgeo import ogr

from ui_widgetProximity import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_proximity.py")

      self.outSelector.setType( self.outSelector.FILE )
      self.outputFormat = Utils.fillRasterOutputFormat()

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.valuesEdit, SIGNAL("textChanged(const QString &)"), self.valuesCheck),
          (self.distUnitsCombo, SIGNAL("currentIndexChanged(int)"), self.distUnitsCheck),
          (self.maxDistSpin, SIGNAL("valueChanged(int)"), self.maxDistCheck),
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
          (self.fixedBufValSpin, SIGNAL("valueChanged(int)"), self.fixedBufValCheck)
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)

  def onLayersChanged(self):
      self.inSelector.setLayers( Utils.LayerRegistry().getRasterLayers() )

  def fillInputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      inputFile = Utils.FileDialog.getOpenFileName(self, self.tr( "Select the input file for Proximity" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
      if not inputFile:
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if not outputFile:
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat( lastUsedFilter, outputFile )
      self.outSelector.setFilename(outputFile)

  def getArguments(self):
      arguments = []
      arguments.append( self.getInputFileName())
      outputFn = self.getOutputFileName()
      arguments.append( outputFn)
      if self.valuesCheck.isChecked():
        values = self.valuesEdit.text().split()
        if values:
          arguments.append( "-values")
          arguments.append( ','.join( values) )
      if self.distUnitsCheck.isChecked() and self.distUnitsCombo.currentIndex() >= 0:
        arguments.append( "-distunits")
        arguments.append( self.distUnitsCombo.currentText())
      if self.maxDistCheck.isChecked():
        arguments.append( "-maxdist")
        arguments.append( str(self.maxDistSpin.value()))
      if self.noDataCheck.isChecked():
        arguments.append( "-nodata")
        arguments.append( str(self.noDataSpin.value()))
      if self.fixedBufValCheck.isChecked():
        arguments.append( "-fixed-buf-val")
        arguments.append( str(self.fixedBufValSpin.value()))
      if outputFn:
        arguments.append( "-of")
        arguments.append( self.outputFormat)
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      return self.inSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

