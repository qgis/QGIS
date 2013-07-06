# -*- coding: utf-8 -*-

"""
***************************************************************************
    doBuildVRT.py
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

      self.inSelector.setType( self.inSelector.FILE )
      self.outSelector.setType( self.outSelector.FILE )
      self.recurseCheck.hide()
      self.visibleRasterLayers = []

      self.setParamsStatus(
        [
          (self.inSelector, SIGNAL("filenameChanged()")),
          (self.outSelector, SIGNAL("filenameChanged()")),
          (self.resolutionComboBox, SIGNAL("currentIndexChanged(int)"), self.resolutionCheck),
          (self.srcNoDataSpin, SIGNAL("valueChanged(int)"), self.srcNoDataCheck, "1.7.0"),
          (self.inputDirCheck, SIGNAL("stateChanged(int)")),
          (self.separateCheck, SIGNAL("stateChanged(int)"), None, "1.7.0"),
          (self.allowProjDiffCheck, SIGNAL("stateChanged(int)"), None, "1.7.0"),
          (self.recurseCheck, SIGNAL("stateChanged(int)"), self.inputDirCheck),
          (self.inputSelLayersCheck, SIGNAL("stateChanged(int)"))
        ]
      )

      self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
      self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)
      self.connect( self.inputDirCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )
      self.connect( self.inputSelLayersCheck, SIGNAL( "stateChanged( int )" ), self.switchLayerMode )
      self.connect( self.iface.mapCanvas(), SIGNAL( "stateChanged( int )" ), self.switchLayerMode )


  def initialize(self):
      # connect to mapCanvas.layerChanged() signal
      self.connect(self.iface.mapCanvas(), SIGNAL("layersChanged()"), self.onVisibleLayersChanged)
      self.onVisibleLayersChanged()
      BasePluginWidget.initialize(self)

  def onClosing(self):
      # disconnect from mapCanvas.layerChanged() signal
      self.disconnect(self.iface.mapCanvas(), SIGNAL("layersChanged()"), self.onVisibleLayersChanged)
      BasePluginWidget.onClosing(self)


  def onVisibleLayersChanged(self):
      # refresh list of visible raster layers
      self.visibleRasterLayers = []
      for layer in self.iface.mapCanvas().layers():
        if Utils.LayerRegistry.isRaster( layer ):
          self.visibleRasterLayers.append( layer.source() )

      # refresh the text in the command viewer
      self.someValueChanged()

  def switchToolMode(self):
      self.recurseCheck.setVisible( self.inputDirCheck.isChecked() )
      self.inSelector.clear()

      if self.inputDirCheck.isChecked():
        self.inFileLabel = self.label.text()
        self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

        QObject.disconnect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
        QObject.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputDir)
      else:
        self.label.setText( self.inFileLabel )

        QObject.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFilesEdit)
        QObject.disconnect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputDir)

  def switchLayerMode(self):
      enableInputFiles = not self.inputSelLayersCheck.isChecked()
      self.inputDirCheck.setEnabled( enableInputFiles )
      self.inSelector.setEnabled( enableInputFiles )
      self.recurseCheck.setEnabled( enableInputFiles )

  def fillInputFilesEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      files = Utils.FileDialog.getOpenFileNames(self, self.tr( "Select the files for VRT" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if files == '':
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
      self.inSelector.setFilename(",".join(files))

  def fillOutputFileEdit(self):
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select where to save the VRT" ), self.tr( "VRT (*.vrt)" ))
      if outputFile == '':
        return
      self.outSelector.setFilename(outputFile)

  def fillInputDir( self ):
      inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files for VRT" ))
      if inputDir == '':
        return
      self.inSelector.setFilename( inputDir )

  def getArguments(self):
      arguments = []
      if self.resolutionCheck.isChecked() and self.resolutionComboBox.currentIndex() >= 0:
        arguments.append("-resolution")
        arguments.append(self.resolutions[self.resolutionComboBox.currentIndex()])
      if self.separateCheck.isChecked():
        arguments.append("-separate")
      if self.srcNoDataCheck.isChecked():
        arguments.append("-srcnodata")
        arguments.append(str(self.srcNoDataSpin.value()))
      if self.allowProjDiffCheck.isChecked():
        arguments.append("-allow_projection_difference")
      arguments.append(self.getOutputFileName())
      if self.inputSelLayersCheck.isChecked():
        arguments.extend(self.visibleRasterLayers)
      elif self.inputDirCheck.isChecked():
        arguments.extend(Utils.getRasterFiles( self.getInputFileName(), self.recurseCheck.isChecked() ))
      else:
        arguments.extend(self.getInputFileName())
      return arguments

  def getOutputFileName(self):
      return self.outSelector.filename()

  def getInputFileName(self):
      if self.inputDirCheck.isChecked():
        return self.inSelector.filename()
      return self.inSelector.filename().split(",")

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())

