# -*- coding: utf-8 -*-

"""
***************************************************************************
    doWarp.py
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

from qgis.PyQt.QtCore import QCoreApplication, QDir
from qgis.PyQt.QtWidgets import QWidget
from qgis.core import QGis

from .ui_widgetWarp import Ui_GdalToolsWidget as Ui_Widget
from .widgetBatchBase import GdalToolsBaseBatchWidget as BaseBatchWidget
from .dialogSRS import GdalToolsSRSDialog as SRSDialog
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BaseBatchWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.resampling_method = ('near', 'bilinear', 'cubic', 'cubicspline', 'lanczos')

        self.setupUi(self)
        BaseBatchWidget.__init__(self, self.iface, "gdalwarp")

        self.outSelector.setType(self.outSelector.FILE)

        # set the default QSpinBoxes and QProgressBar value
        self.widthSpin.setValue(3000)
        self.heightSpin.setValue(3000)
        self.progressBar.setValue(0)

        self.progressBar.hide()

        self.outputFormat = Utils.fillRasterOutputFormat()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.sourceSRSEdit, "textChanged", self.sourceSRSCheck),
            (self.selectSourceSRSButton, None, self.sourceSRSCheck),
            (self.targetSRSEdit, "textChanged", self.targetSRSCheck),
            (self.selectTargetSRSButton, None, self.targetSRSCheck),
            (self.resamplingCombo, "currentIndexChanged", self.resamplingCheck),
            (self.cacheSpin, "valueChanged", self.cacheCheck),
            ([self.widthSpin, self.heightSpin], "valueChanged", self.resizeGroupBox),
            (self.multithreadCheck, "stateChanged"),
            (self.noDataEdit, "textChanged", self.noDataCheck),
            (self.maskSelector, "filenameChanged", self.maskCheck, 1600),
        ])

        self.inSelector.layerChanged.connect(self.fillSourceSRSEditDefault)
        self.inSelector.selectClicked.connect(self.fillInputFile)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.selectSourceSRSButton.clicked.connect(self.fillSourceSRSEdit)
        self.selectTargetSRSButton.clicked.connect(self.fillTargetSRSEdit)
        self.maskSelector.selectClicked.connect(self.fillMaskFile)
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
        self.maskSelector.setLayers([x for x in Utils.LayerRegistry.instance().getVectorLayers() if x.geometryType() == QGis.Polygon])

    def fillInputFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Warp"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.inSelector.setFilename(inputFile)

        # get SRS for source file if necessary and possible
        self.refreshSourceSRS()

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillMaskFile(self):
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        maskFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the mask file"), Utils.FileFilter.allVectorsFilter(), lastUsedFilter)
        if not maskFile:
            return
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)
        self.maskSelector.setFilename(maskFile)

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files to Warp"))
        if not inputDir:
            return

        self.inSelector.setFilename(inputDir)

        filter = Utils.getRasterExtensions()
        workDir = QDir(inputDir)
        workDir.setFilter(QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot)
        workDir.setNameFilters(filter)
        if len(workDir.entryList()) > 0:
            fl = inputDir + "/" + workDir.entryList()[0]
            self.sourceSRSEdit.setText(Utils.getRasterSRS(self, fl))

    def fillOutputDir(self):
        outputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the output directory to save the results to"))
        if not outputDir:
            return
        self.outSelector.setFilename(outputDir)

    def fillSourceSRSEdit(self):
        dialog = SRSDialog("Select the source SRS", self)
        if dialog.exec_():
            self.sourceSRSEdit.setText(dialog.getProjection())

    def fillSourceSRSEditDefault(self):
        if self.inSelector.layer() is None:
            return
        self.refreshSourceSRS()

    def refreshSourceSRS(self):
        crs = Utils.getRasterSRS(self, self.getInputFileName())
        self.sourceSRSEdit.setText(crs)
        self.sourceSRSCheck.setChecked(crs != '')

    def fillTargetSRSEdit(self):
        dialog = SRSDialog("Select the target SRS", self)
        if dialog.exec_():
            self.targetSRSEdit.setText(dialog.getProjection())

    def getArguments(self):
        arguments = []
        if not self.isBatchEnabled():
            arguments.append("-overwrite")
        if self.sourceSRSCheck.isChecked() and self.sourceSRSEdit.text():
            arguments.append("-s_srs")
            arguments.append(self.sourceSRSEdit.text())
        if self.targetSRSCheck.isChecked() and self.targetSRSEdit.text():
            arguments.append("-t_srs")
            arguments.append(self.targetSRSEdit.text())
        if self.resamplingCheck.isChecked() and self.resamplingCombo.currentIndex() >= 0:
            arguments.append("-r")
            arguments.append(self.resampling_method[self.resamplingCombo.currentIndex()])
        if self.cacheCheck.isChecked():
            arguments.append("-wm")
            arguments.append(unicode(self.cacheSpin.value()))
        if self.resizeGroupBox.isChecked():
            arguments.append("-ts")
            arguments.append(unicode(self.widthSpin.value()))
            arguments.append(unicode(self.heightSpin.value()))
        if self.multithreadCheck.isChecked():
            arguments.append("-multi")
        if self.noDataCheck.isChecked():
            nodata = self.noDataEdit.text().strip()
            if nodata:
                arguments.append("-dstnodata")
                arguments.append(nodata)
        if self.maskCheck.isChecked():
            mask = self.getMaskFileName()
            if mask:
                arguments.append("-q")
                arguments.append("-cutline")
                arguments.append(mask)
                arguments.append("-dstalpha")
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

    def getMaskFileName(self):
        return self.maskSelector.filename()

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
