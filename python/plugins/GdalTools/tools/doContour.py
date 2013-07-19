# -*- coding: utf-8 -*-

"""
***************************************************************************
    doContour.py
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

from ui_widgetContour import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_contour")

      gdalVersion = Utils.GdalConfig.versionNum()
      self.useDirAsOutput = gdalVersion < 1700
      if self.useDirAsOutput:
	      self.label_2.setText( QApplication.translate("GdalToolsWidget", "&Output directory for contour lines (shapefile)") )

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
      if not inputFile:
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.inSelector.setFilename(inputFile)

  def fillOutputFileEdit(self):
      if not self.useDirAsOutput:
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        outputFile, encoding = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the Contour output" ), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
      else:
        outputFile, encoding = Utils.FileDialog.getExistingDirectory(self, self.tr( "Select where to save the Contour output" ), True)

      if not outputFile:
        return

      if not self.useDirAsOutput:
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

      self.outSelector.setFilename(outputFile)
      self.lastEncoding = encoding

  def getArguments(self):
      arguments = []
      if self.attributeCheck.isChecked() and self.attributeEdit.text():
        arguments.append("-a")
        arguments.append(self.attributeEdit.text())
      if True: # XXX in this moment the -i argument is not optional
        arguments.append("-i")
        arguments.append(str(self.intervalDSpinBox.value()))
      arguments.append(self.getInputFileName())
      arguments.append(self.outSelector.filename())
      return arguments

  def getInputFileName(self):
      return self.inSelector.filename()

  def getOutputFileName(self):
      if self.useDirAsOutput:
        if self.outSelector.filename():
          return self.outSelector.filename() + QDir.separator() + "contour.shp"
      return self.outSelector.filename()

  def addLayerIntoCanvas(self, fileInfo):
      vl = self.iface.addVectorLayer(fileInfo.filePath(), fileInfo.baseName(), "ogr")
      if vl != None and vl.isValid():
        if hasattr(self, 'lastEncoding'):
          vl.setProviderEncoding(self.lastEncoding)
