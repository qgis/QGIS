# -*- coding: utf-8 -*-

"""
***************************************************************************
    doDEM.py
    ---------------------
    Date                 : March 2011
    Copyright            : (C) 2011 by Giuseppe Sucameli
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
__date__ = 'March 2011'
__copyright__ = '(C) 2011, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QWidget

from .ui_widgetDEM import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.modes = ("hillshade", "slope", "aspect", "color-relief", "TRI", "TPI", "roughness")

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdaldem")

        self.outSelector.setType(self.outSelector.FILE)
        self.configSelector.setType(self.configSelector.FILE)

        # set the default QSpinBoxes and QProgressBar value
        self.bandSpin.setValue(1)

        self.hillshadeZFactorSpin.setValue(1)
        self.hillshadeScaleSpin.setValue(1)
        self.hillshadeAltitudeSpin.setValue(45.0)
        self.hillshadeAzimuthSpin.setValue(315.0)
        self.slopeScaleSpin.setValue(1)

        # set the default color configuration file to terrain
        import os.path
        colorConfigFile = os.path.join(os.path.dirname(__file__), "terrain.txt")
        self.configSelector.setFilename(colorConfigFile)

        self.outputFormat = Utils.fillRasterOutputFormat()
        self.creationOptionsWidget.setFormat(self.outputFormat)

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.computeEdgesCheck, "stateChanged", None, 1800),
            (self.bandSpin, "valueChanged", self.bandCheck),
            (self.algorithmCheck, "stateChanged", None, 1800),
            (self.creationOptionsWidget, "optionsChanged"),
            (self.creationOptionsGroupBox, "toggled"),
            (self.modeCombo, "currentIndexChanged"),
            ([self.hillshadeZFactorSpin, self.hillshadeScaleSpin, self.hillshadeAltitudeSpin, self.hillshadeAzimuthSpin], "valueChanged"),
            (self.slopeScaleSpin, "valueChanged"),
            (self.slopePercentCheck, "stateChanged"),
            ([self.aspectTrigonometricCheck, self.aspectZeroForFlatCheck], "stateChanged"),
            (self.configSelector, "filenameChanged"),
            ([self.colorExactRadio, self.colorNearestRadio], "toggled", self.colorMatchGroupBox),
            (self.colorAlphaCheck, "stateChanged")
        ])

        self.inSelector.selectClicked.connect(self.fillInputFileEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.configSelector.selectClicked.connect(self.fillColorConfigFileEdit)
        self.modeCombo.currentIndexChanged.connect(self.showModeParams)

    def showModeParams(self, index):
        self.stackedWidget.setVisible(index < 4)
        self.algorithmCheck.setVisible(index < 3)
        if index >= 3:
            self.algorithmCheck.setChecked(False)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the file for DEM"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)
        self.getArguments()

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)
        self.creationOptionsWidget.setFormat(self.outputFormat)

    def fillColorConfigFileEdit(self):
        configFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the color configuration file"))
        if not configFile:
            return
        self.configSelector.setFilename(configFile)

    def getArguments(self):
        mode = self.modes[self.modeCombo.currentIndex()]
        arguments = []
        arguments.append(mode)
        arguments.append(self.getInputFileName())
        if mode == "color-relief":
            arguments.append(self.configSelector.filename())
        outputFn = self.getOutputFileName()
        arguments.append(outputFn)
        if mode == "hillshade":
            arguments.extend(["-z", self.hillshadeZFactorSpin.value()])
            arguments.extend(["-s", self.hillshadeScaleSpin.value()])
            arguments.extend(["-az", self.hillshadeAzimuthSpin.value()])
            arguments.extend(["-alt", self.hillshadeAltitudeSpin.value()])
        elif mode == "slope":
            if self.slopePercentCheck.isChecked():
                arguments.append("-p")
            arguments.extend(["-s", self.slopeScaleSpin.value()])
        elif mode == "aspect":
            if self.aspectTrigonometricCheck.isChecked():
                arguments.append("-trigonometric")
            if self.aspectZeroForFlatCheck.isChecked():
                arguments.append("-zero_for_flat")
        elif mode == "color-relief":
            if self.colorAlphaCheck.isChecked():
                arguments.append("-alpha")
            if self.colorMatchGroupBox.isChecked():
                if self.colorExactRadio.isChecked():
                    arguments.append("-exact_color_entry")
                elif self.colorNearestRadio.isChecked():
                    arguments.append("-nearest_color_entry")
        if self.algorithmCheck.isChecked():
            arguments.extend(["-alg", "ZevenbergenThorne"])
        if self.computeEdgesCheck.isChecked():
            arguments.append("-compute_edges")
        if self.bandCheck.isChecked():
            arguments.extend(["-b", self.bandSpin.value()])
        if outputFn:
            arguments.extend(["-of", self.outputFormat])
        if self.creationOptionsGroupBox.isChecked():
            for opt in self.creationOptionsWidget.options():
                arguments.extend(["-co", opt])
        # set creation options filename/layer for validation
        if self.inSelector.layer():
            self.creationOptionsWidget.setRasterLayer(self.inSelector.layer())
        else:
            self.creationOptionsWidget.setRasterFileName(self.getInputFileName())

        return arguments

    def getInputFileName(self):
        return self.inSelector.filename()

    def getOutputFileName(self):
        return self.outSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())
