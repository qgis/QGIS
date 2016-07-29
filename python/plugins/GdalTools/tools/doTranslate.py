# -*- coding: utf-8 -*-

"""
***************************************************************************
    doTranslate.py
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

from qgis.PyQt.QtCore import Qt, QCoreApplication, QDir
from qgis.PyQt.QtWidgets import QWidget, QMessageBox

from .ui_widgetTranslate import Ui_GdalToolsWidget as Ui_Widget
from .widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from .dialogSRS import GdalToolsSRSDialog as SRSDialog
from . import GdalTools_utils as Utils

import re


class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.canvas = self.iface.mapCanvas()
        self.expand_method = ('gray', 'rgb', 'rgba')

        self.setupUi(self)
        BaseBatchWidget.__init__(self, self.iface, "gdal_translate")

        self.outSelector.setType(self.outSelector.FILE)

        # set the default QSpinBoxes and QProgressBar value
        self.outsizeSpin.setValue(25)
        self.progressBar.setValue(0)

        self.progressBar.hide()
        self.formatLabel.hide()
        self.formatCombo.hide()

        if Utils.GdalConfig.versionNum() < 1700:
            index = self.expandCombo.findText('gray', Qt.MatchFixedString)
            if index >= 0:
                self.expandCombo.removeItem(index)

        self.outputFormat = Utils.fillRasterOutputFormat()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.targetSRSEdit, "textChanged", self.targetSRSCheck),
            (self.selectTargetSRSButton, None, self.targetSRSCheck),
            (self.creationOptionsWidget, "optionsChanged"),
            (self.outsizeSpin, "valueChanged", self.outsizeCheck),
            (self.nodataSpin, "valueChanged", self.nodataCheck),
            (self.expandCombo, "currentIndexChanged", self.expandCheck, 1600),
            (self.sdsCheck, "stateChanged"),
            (self.srcwinEdit, "textChanged", self.srcwinCheck),
            (self.prjwinEdit, "textChanged", self.prjwinCheck)
        ])

        #self.canvas.layersChanged.connect(self.fillInputLayerCombo)
        self.inSelector.layerChanged.connect(self.fillTargetSRSEditDefault)
        self.inSelector.selectClicked.connect(self.fillInputFile)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.selectTargetSRSButton.clicked.connect(self.fillTargetSRSEdit)
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
            self.inFileLabel = self.label_3.text()
            self.outFileLabel = self.label_2.text()
            self.label_3.setText(QCoreApplication.translate("GdalTools", "&Input directory"))
            self.label_2.setText(QCoreApplication.translate("GdalTools", "&Output directory"))

            self.inSelector.selectClicked.disconnect(self.fillInputFile)
            self.outSelector.selectClicked.disconnect(self.fillOutputFileEdit)

            self.inSelector.selectClicked.connect(self. fillInputDir)
            self.outSelector.selectClicked.connect(self.fillOutputDir)
        else:
            self.label_3.setText(self.inFileLabel)
            self.label_2.setText(self.outFileLabel)

            self.inSelector.selectClicked.disconnect(self.fillInputDir)
            self.outSelector.selectClicked.disconnect(self.fillOutputDir)

            self.inSelector.selectClicked.connect(self.fillInputFile)
            self.outSelector.selectClicked.connect(self.fillOutputFileEdit)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

    def fillInputFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Translate"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)

        # get SRS for target file if necessary and possible
        self.refreshTargetSRS()

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files to Translate"))
        if not inputDir:
            return
        self.inSelector.setFilename(inputDir)

        filter = Utils.getRasterExtensions()
        workDir = QDir(inputDir)
        workDir.setFilter(QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot)
        workDir.setNameFilters(filter)

        # search for a valid SRS, then use it as default target SRS
        srs = ''
        for fname in workDir.entryList():
            fl = inputDir + "/" + fname
            srs = Utils.getRasterSRS(self, fl)
            if srs:
                break
        self.targetSRSEdit.setText(srs)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillOutputDir(self):
        outputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the output directory to save the results to"))
        if not outputDir:
            return
        self.outSelector.setFilename(outputDir)

    def fillTargetSRSEditDefault(self):
        if self.inSelector.layer() is None:
            return
        self.refreshTargetSRS()

    def refreshTargetSRS(self):
        self.targetSRSEdit.setText(Utils.getRasterSRS(self, self.getInputFileName()))

    def fillTargetSRSEdit(self):
        dialog = SRSDialog("Select the target SRS", self)
        if dialog.exec_():
            self.targetSRSEdit.setText(dialog.getProjection())

    def getArguments(self):
        arguments = []
        if self.targetSRSCheck.isChecked() and self.targetSRSEdit.text():
            arguments.append("-a_srs")
            arguments.append(self.targetSRSEdit.text())
        if self.creationOptionsGroupBox.isChecked():
            for opt in self.creationOptionsWidget.options():
                arguments.extend(["-co", opt])
        if self.outsizeCheck.isChecked() and self.outsizeSpin.value() != 100:
            arguments.append("-outsize")
            arguments.append(self.outsizeSpin.text())
            arguments.append(self.outsizeSpin.text())
        if self.expandCheck.isChecked():
            arguments.append("-expand")
            arguments.append(self.expand_method[self.expandCombo.currentIndex()])
        if self.nodataCheck.isChecked():
            arguments.append("-a_nodata")
            arguments.append(unicode(self.nodataSpin.value()))
        if self.sdsCheck.isChecked():
            arguments.append("-sds")
        if self.srcwinCheck.isChecked() and self.srcwinEdit.text():
            coordList = self.srcwinEdit.text().split()  # split the string on whitespace(s)
            if len(coordList) == 4 and coordList[3]:
                try:
                    for x in coordList:
                        int(x)
                except ValueError:
                    #print "Coordinates must be integer numbers."
                    QMessageBox.critical(self, self.tr("Translate - srcwin"), self.tr("Image coordinates (pixels) must be integer numbers."))
                else:
                    arguments.append("-srcwin")
                    for x in coordList:
                        arguments.append(x)
        if self.prjwinCheck.isChecked() and self.prjwinEdit.text():
            coordList = self.prjwinEdit.text().split()  # split the string on whitespace(s)
            if len(coordList) == 4 and coordList[3]:
                try:
                    for x in coordList:
                        float(x)
                except ValueError:
                    #print "Coordinates must be integer numbers."
                    QMessageBox.critical(self, self.tr("Translate - prjwin"), self.tr("Image coordinates (geographic) must be numbers."))
                else:
                    arguments.append("-projwin")
                    for x in coordList:
                        arguments.append(x)
        if self.isBatchEnabled():
            if self.formatCombo.currentIndex() != 0:
                arguments.append("-of")
                arguments.append(Utils.fillRasterOutputFormat(self.formatCombo.currentText()))
                return arguments
            else:
                return arguments

        outputFn = self.getOutputFileName()
        if outputFn:
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(self.getInputFileName())
        arguments.append(outputFn)

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

        filter = Utils.getRasterExtensions()
        workDir = QDir(inDir)
        workDir.setFilter(QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot)
        workDir.setNameFilters(filter)
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
