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

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QWidget

from .ui_widgetBuildVRT import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from .dialogSRS import GdalToolsSRSDialog as SRSDialog
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.resolutions = ("highest", "average", "lowest")

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdalbuildvrt")

        self.inSelector.setType(self.inSelector.FILE)
        self.outSelector.setType(self.outSelector.FILE)
        self.recurseCheck.hide()
        self.visibleRasterLayers = []

        self.setParamsStatus(
            [
                (self.inSelector, "filenameChanged"),
                (self.outSelector, "filenameChanged"),
                (self.resolutionComboBox, "currentIndexChanged", self.resolutionCheck),
                (self.noDataEdit, "textChanged", self.srcNoDataCheck, 1700),
                (self.inputDirCheck, "stateChanged"),
                (self.separateCheck, "stateChanged", None, 1700),
                (self.targetSRSEdit, "textChanged", self.targetSRSCheck),
                (self.allowProjDiffCheck, "stateChanged", None, 1700),
                (self.recurseCheck, "stateChanged", self.inputDirCheck),
                (self.inputSelLayersCheck, "stateChanged")
            ]
        )

        self.inSelector.selectClicked.connect(self.fillInputFilesEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.inputDirCheck.stateChanged.connect(self.switchToolMode)
        self.inputSelLayersCheck.stateChanged.connect(self.switchLayerMode)
        self.iface.mapCanvas().layersChanged.connect(self.switchLayerMode)
        self.selectTargetSRSButton.clicked.connect(self.fillTargetSRSEdit)

    def initialize(self):
        # connect to mapCanvas.layerChanged() signal
        self.iface.mapCanvas().layersChanged.connect(self.onVisibleLayersChanged)
        self.onVisibleLayersChanged()
        BasePluginWidget.initialize(self)

    def onClosing(self):
        # disconnect from mapCanvas.layerChanged() signal
        self.iface.mapCanvas().layersChanged.disconnect(self.onVisibleLayersChanged)
        BasePluginWidget.onClosing(self)

    def onVisibleLayersChanged(self):
        # refresh list of visible raster layers
        self.visibleRasterLayers = []
        for layer in self.iface.mapCanvas().layers():
            if Utils.LayerRegistry.isRaster(layer):
                self.visibleRasterLayers.append(layer.source())

        # refresh the text in the command viewer
        self.someValueChanged()

    def switchToolMode(self):
        self.recurseCheck.setVisible(self.inputDirCheck.isChecked())
        self.inSelector.clear()

        if self.inputDirCheck.isChecked():
            self.inFileLabel = self.label.text()
            self.label.setText(QCoreApplication.translate("GdalTools", "&Input directory"))

            self.inSelector.selectClicked.disconnect(self.fillInputFilesEdit)
            self.inSelector.selectClicked.connect(self.fillInputDir)
        else:
            self.label.setText(self.inFileLabel)

            self.inSelector.selectClicked.connect(self.fillInputFilesEdit)
            self.inSelector.selectClicked.disconnect(self.fillInputDir)

    def switchLayerMode(self):
        enableInputFiles = not self.inputSelLayersCheck.isChecked()
        self.inputDirCheck.setEnabled(enableInputFiles)
        self.inSelector.setEnabled(enableInputFiles)
        self.recurseCheck.setEnabled(enableInputFiles)

    def fillInputFilesEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        files = Utils.FileDialog.getOpenFileNames(self, self.tr("Select the files for VRT"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if files == '':
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.inSelector.setFilename(",".join(files))

    def fillOutputFileEdit(self):
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select where to save the VRT"), self.tr("VRT (*.vrt)"))
        if outputFile == '':
            return
        self.outSelector.setFilename(outputFile)

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files for VRT"))
        if inputDir == '':
            return
        self.inSelector.setFilename(inputDir)

    def fillTargetSRSEdit(self):
        dialog = SRSDialog("Select the target SRS", self)
        if dialog.exec_():
            self.targetSRSEdit.setText(dialog.getProjection())

    def getArguments(self):
        arguments = []
        if self.resolutionCheck.isChecked() and self.resolutionComboBox.currentIndex() >= 0:
            arguments.append("-resolution")
            arguments.append(self.resolutions[self.resolutionComboBox.currentIndex()])
        if self.separateCheck.isChecked():
            arguments.append("-separate")
        if self.srcNoDataCheck.isChecked():
            nodata = self.noDataEdit.text().strip()
            if nodata:
                arguments.append("-srcnodata")
                arguments.append(nodata)
        if self.targetSRSCheck.isChecked() and self.targetSRSEdit.text():
            arguments.append("-a_srs")
            arguments.append(self.targetSRSEdit.text())
        if self.allowProjDiffCheck.isChecked():
            arguments.append("-allow_projection_difference")
        arguments.append(self.getOutputFileName())
        if self.inputSelLayersCheck.isChecked():
            arguments.extend(self.visibleRasterLayers)
        elif self.inputDirCheck.isChecked():
            arguments.extend(Utils.getRasterFiles(self.getInputFileName(), self.recurseCheck.isChecked()))
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
