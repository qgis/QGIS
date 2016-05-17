# -*- coding: utf-8 -*-

"""
***************************************************************************
    doPctRgb.py
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

from .ui_widgetConvert import Ui_GdalToolsWidget as Ui_Widget
from .widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface

        self.setupUi(self)
        BaseBatchWidget.__init__(self, self.iface, "pct2rgb.py")

        # we use one widget for two tools
        self.base.setWindowTitle(self.tr("Convert paletted image to RGB"))

        self.outSelector.setType(self.outSelector.FILE)

        # set the default QSpinBoxes and QProgressBar value
        self.bandSpin.setValue(1)
        self.progressBar.setValue(0)

        self.progressBar.hide()
        self.outputFormat = Utils.fillRasterOutputFormat()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.colorsSpin, "valueChanged", self.colorsCheck, "-1"),   # hide this option
            (self.bandSpin, "valueChanged", self.bandCheck)
        ])

        self.inSelector.selectClicked.connect(self.fillInputFile)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.batchCheck.stateChanged.connect(self.switchToolMode)

    # switch to batch or normal mode
    def switchToolMode(self):
        self.setCommandViewerEnabled(not self.batchCheck.isChecked())
        self.progressBar.setVisible(self.batchCheck.isChecked())

        self.inSelector.setType(self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER)
        self.outSelector.clear()

        if self.batchCheck.isChecked():
            self.inFileLabel = self.label.text()
            self.outFileLabel = self.label_2.text()
            self.label.setText(QCoreApplication.translate("GdalTools", "&Input directory"))
            self.label_2.setText(QCoreApplication.translate("GdalTools", "&Output directory"))

            self.inSelector.selectClicked.disconnect(self.fillInputFile)
            self.outSelector.selectClicked.disconnect(self.fillOutputFileEdit)

            self.inSelector.selectClicked.connect(self.fillInputDir)
            self.outSelector.selectClicked.connect(self.fillOutputDir)
        else:
            self.label.setText(self.inFileLabel)
            self.label_2.setText(self.outFileLabel)

            self.inSelector.selectClicked.disconnect(self.fillInputDir)
            self.outSelector.selectClicked.disconnect(self.fillOutputDir)

            self.inSelector.selectClicked.connect(self.fillInputFile)
            self.outSelector.selectClicked.connect(self.fillOutputFileEdit)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

    def fillInputFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for convert"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.inSelector.setFilename(inputFile)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files for convert"))
        if not inputDir:
            return
        self.inSelector.setFilename(inputDir)

    def fillOutputDir(self):
        outputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the output directory to save the results to"))
        if not outputDir:
            return
        self.outSelector.setFilename(outputDir)

    def getArguments(self):
        arguments = []
        if self.bandCheck.isChecked():
            arguments.append("-b")
            arguments.append(unicode(self.bandSpin.value()))
        if self.isBatchEnabled():
            return arguments

        outputFn = self.getOutputFileName()
        if outputFn:
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(self.getInputFileName())
        arguments.append(outputFn)
        return arguments

    def getInputFileName(self):
        return self.inSelector.filename()

    def getOutputFileName(self):
        return self.outSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())

    def isBatchEnabled(self):
        return self.batchCheck.isChecked()

    def setProgressRange(self, maximum):
        self.progressBar.setRange(0, maximum)

    def updateProgress(self, index, total):
        if index < total:
            self.progressBar.setValue(index + 1)
        else:
            self.progressBar.setValue(0)
