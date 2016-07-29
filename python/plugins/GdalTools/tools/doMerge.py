# -*- coding: utf-8 -*-

"""
***************************************************************************
    doMerge.py
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
from qgis.PyQt.QtWidgets import QWidget, QMessageBox

from .ui_widgetMerge import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_merge.py")

        self.inSelector.setType(self.inSelector.FILE)
        self.outSelector.setType(self.outSelector.FILE)
        self.recurseCheck.hide()
        # use this for approx. previous UI
        #self.creationOptionsWidget.setType(QgsRasterFormatSaveOptionsWidget.Table)

        self.outputFormat = Utils.fillRasterOutputFormat()
        self.extent = None

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.noDataSpin, "valueChanged", self.noDataCheck),
            (self.inputDirCheck, "stateChanged"),
            (self.recurseCheck, "stateChanged", self.inputDirCheck),
            (self.separateCheck, "stateChanged"),
            (self.pctCheck, "stateChanged"),
            (self.intersectCheck, "stateChanged"),
            (self.creationOptionsWidget, "optionsChanged"),
            (self.creationOptionsGroupBox, "toggled")
        ])

        self.inSelector.selectClicked.connect(self.fillInputFilesEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.intersectCheck.toggled.connect(self.refreshExtent)
        self.inputDirCheck.stateChanged.connect(self.switchToolMode)
        self.inSelector.filenameChanged.connect(self.refreshExtent)

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

    def fillInputFilesEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        files = Utils.FileDialog.getOpenFileNames(self, self.tr("Select the files to Merge"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not files:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)
        self.inSelector.setFilename(files)

    def refreshExtent(self):
        files = self.getInputFileNames()
        self.intersectCheck.setEnabled(len(files) > 1)

        if not self.intersectCheck.isChecked():
            self.someValueChanged()
            return

        if len(files) < 2:
            self.intersectCheck.setChecked(False)
            return

        self.extent = self.getIntersectedExtent(files)

        if self.extent is None:
            QMessageBox.warning(self, self.tr("Error retrieving the extent"), self.tr('GDAL was unable to retrieve the extent from any file. \nThe "Use intersected extent" option will be unchecked.'))
            self.intersectCheck.setChecked(False)
            return

        elif self.extent.isEmpty():
            QMessageBox.warning(self, self.tr("Empty extent"), self.tr('The computed extent is empty. \nDisable the "Use intersected extent" option to have a nonempty output.'))

        self.someValueChanged()

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select where to save the Merge output"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillInputDir(self):
        inputDir = Utils.FileDialog.getExistingDirectory(self, self.tr("Select the input directory with files to Merge"))
        if not inputDir:
            return
        self.inSelector.setFilename(inputDir)

    def getArguments(self):
        arguments = []
        if self.intersectCheck.isChecked():
            if self.extent is not None:
                arguments.append("-ul_lr")
                arguments.append(unicode(self.extent.xMinimum()))
                arguments.append(unicode(self.extent.yMaximum()))
                arguments.append(unicode(self.extent.xMaximum()))
                arguments.append(unicode(self.extent.yMinimum()))
        if self.noDataCheck.isChecked():
            arguments.append("-n")
            arguments.append(unicode(self.noDataSpin.value()))
            if Utils.GdalConfig.versionNum() >= 1900:
                arguments.append("-a_nodata")
                arguments.append(unicode(self.noDataSpin.value()))
        if self.separateCheck.isChecked():
            arguments.append("-separate")
        if self.pctCheck.isChecked():
            arguments.append("-pct")
        if self.creationOptionsGroupBox.isChecked():
            for opt in self.creationOptionsWidget.options():
                arguments.append("-co")
                arguments.append(opt)
        outputFn = self.getOutputFileName()
        if outputFn:
            arguments.append("-of")
            arguments.append(self.outputFormat)
            arguments.append("-o")
            arguments.append(outputFn)
        arguments.extend(self.getInputFileNames())
        return arguments

    def getOutputFileName(self):
        return self.outSelector.filename()

    def getInputFileName(self):
        if self.inputDirCheck.isChecked():
            return self.inSelector.filename()
        return self.inSelector.filename().split(",")

    def getInputFileNames(self):
        if self.inputDirCheck.isChecked():
            return Utils.getRasterFiles(self.inSelector.filename(), self.recurseCheck.isChecked())
        return self.inSelector.filename().split(",")

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())

    def getIntersectedExtent(self, files):
        res = None
        for fileName in files:
            if res is None:
                res = Utils.getRasterExtent(self, fileName)
                continue

            rect2 = Utils.getRasterExtent(self, fileName)
            if rect2 is None:
                continue

            res = res.intersect(rect2)
            if res.isEmpty():
                break

        return res
