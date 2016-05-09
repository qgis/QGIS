# -*- coding: utf-8 -*-

"""
***************************************************************************
    doFillNodata.py
    ---------------------
    Date                 : November 2011
    Copyright            : (C) 2011 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'November 2011'
__copyright__ = '(C) 2011, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import Qt, QCoreApplication, QDir
from qgis.PyQt.QtWidgets import QWidget

from .ui_widgetFillNodata import Ui_GdalToolsWidget as Ui_Widget
from .widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from . import GdalTools_utils as Utils

import re


class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface

        self.setupUi(self)
        BaseBatchWidget.__init__(self, self.iface, "gdal_fillnodata.py")

        self.inSelector.setType(self.inSelector.FILE_LAYER)
        self.outSelector.setType(self.outSelector.FILE)
        self.maskSelector.setType(self.maskSelector.FILE)

        self.progressBar.setValue(0)
        self.progressBar.hide()
        self.formatLabel.hide()
        self.formatCombo.hide()

        self.outputFormat = Utils.fillRasterOutputFormat()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.maskSelector, "filenameChanged", self.maskCheck),
            (self.distanceSpin, "valueChanged", self.distanceCheck),
            (self.smoothSpin, "valueChanged", self.smoothCheck),
            (self.bandSpin, "valueChanged", self.bandCheck),
            (self.nomaskCheck, "stateChanged")
        ])

        self.inSelector.selectClicked.connect(self.fillInputFile)
        self.outSelector.selectClicked.connect(self.fillOutputFile)
        self.maskSelector.selectClicked.connect(self.fillMaskFile)
        self.batchCheck.stateChanged.connect(self.switchToolMode)

        # add raster filters to combo
        self.formatCombo.addItems(Utils.FileFilter.allRastersFilter().split(";;"))

    def switchToolMode(self):
        self.setCommandViewerEnabled(not self.batchCheck.isChecked())
        self.progressBar.setVisible(self.batchCheck.isChecked())
        self.formatLabel.setVisible(self.batchCheck.isChecked())
        self.formatCombo.setVisible(self.batchCheck.isChecked())

        self.inSelector.setType(self.inSelector.FILE if self.batchCheck.isChecked() else self.inSelector.FILE_LAYER)
        self.outSelector.clear()

        if self.batchCheck.isChecked():
            self.inFileLabel = self.label.text()
            self.outFileLabel = self.label_1.text()
            self.label.setText(QCoreApplication.translate("GdalTools", "&Input directory"))
            self.label_1.setText(QCoreApplication.translate("GdalTools", "&Output directory"))

            self.inSelector.selectClicked.disconnect(self.fillInputFile)
            self.outSelector.selectClicked.disconnect(self.fillOutputFile)

            self.inSelector.selectClicked.connect(self. fillInputDir)
            self.outSelector.selectClicked.connect(self.fillOutputDir)
        else:
            self.label.setText(self.inFileLabel)
            self.label_1.setText(self.outFileLabel)

            self.inSelector.selectClicked.disconnect(self.fillInputDir)
            self.outSelector.selectClicked.disconnect(self.fillOutputDir)

            self.inSelector.selectClicked.connect(self.fillInputFile)
            self.outSelector.selectClicked.connect(self.fillOutputFile)

    def fillInputFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self,
                                                     self.tr("Select the files to analyse"),
                                                     Utils.FileFilter.allRastersFilter(),
                                                     lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.inSelector.setFilename(inputFile)

    def fillOutputFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillMaskFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self,
                                                     self.tr("Select the files to analyse"),
                                                     Utils.FileFilter.allRastersFilter(),
                                                     lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.maskSelector.setFilename(inputFile)

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files"))
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
        maskFile = self.maskSelector.filename()
        if self.distanceCheck.isChecked() and self.distanceSpin.value() != 0:
            arguments.append("-md")
            arguments.append(self.distanceSpin.text())
        if self.smoothCheck.isChecked() and self.smoothSpin.value() != 0:
            arguments.append("-si")
            arguments.append(unicode(self.smoothSpin.value()))
        if self.bandCheck.isChecked() and self.bandSpin.value() != 0:
            arguments.append("-b")
            arguments.append(unicode(self.bandSpin.value()))
        if self.maskCheck.isChecked() and maskFile:
            arguments.append("-mask")
            arguments.append(maskFile)
        if self.nomaskCheck.isChecked():
            arguments.append("-nomask")
        if self.isBatchEnabled():
            if self.formatCombo.currentIndex() != 0:
                arguments.append("-of")
                arguments.append(Utils.fillRasterOutputFormat(self.formatCombo.currentText()))
            return arguments
        else:
            outputFn = self.getOutputFileName()
            if outputFn:
                arguments.append("-of")
                arguments.append(self.outputFormat)
            arguments.append(self.getInputFileName())
            arguments.append(outputFn)
            return arguments

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

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

    def batchRun(self):
        exts = re.sub('\).*$', '', re.sub('^.*\(', '', self.formatCombo.currentText())).split(" ")
        if len(exts) > 0 and exts[0] != "*" and exts[0] != "*.*":
            outExt = exts[0].replace("*", "")
        else:
            outExt = ".tif"

        self.base.enableRun(False)
        self.base.setCursor(Qt.WaitCursor)

        inDir = self.getInputFileName()
        outDir = self.getOutputFileName()

        extensions = Utils.getRasterExtensions()
        workDir = QDir(inDir)
        workDir.setFilter(QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot)
        workDir.setNameFilters(extensions)
        files = workDir.entryList()

        self.inFiles = []
        self.outFiles = []

        for f in files:
            self.inFiles.append(inDir + "/" + f)
            if outDir is not None:
                outFile = re.sub("\.[a-zA-Z0-9]{2,4}", outExt, f)
                self.outFiles.append(outDir + "/" + outFile)

        self.errors = []
        self.batchIndex = 0
        self.batchTotal = len(self.inFiles)
        self.setProgressRange(self.batchTotal)

        self.runItem(self.batchIndex, self.batchTotal)
