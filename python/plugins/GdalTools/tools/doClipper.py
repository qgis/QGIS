# -*- coding: utf-8 -*-

"""
***************************************************************************
    doClipper.py
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

from qgis.PyQt.QtWidgets import QWidget
from qgis.core import QGis

from .ui_widgetClipper import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.canvas = self.iface.mapCanvas()

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_translate")

        self.outSelector.setType(self.outSelector.FILE)
        self.extentSelector.setCanvas(self.canvas)
        self.outputFormat = Utils.fillRasterOutputFormat()

        # set the default QDoubleSpinBoxes
        self.xRes.setValue(12.5)
        self.yRes.setValue(12.5)

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.noDataSpin, "valueChanged", self.noDataCheck, 1700),
            (self.maskSelector, "filenameChanged", self.maskModeRadio, 1600),
            (self.alphaBandCheck, "stateChanged"),
            (self.cropToCutlineCheck, "stateChanged"),
            ([self.xRes, self.yRes], "valueChanged", self.setResolutionRadio),
            (self.extentSelector, ["selectionStarted", "newExtentDefined"], self.extentModeRadio),
            (self.modeStackedWidget, "currentChanged")
        ])

        self.inSelector.selectClicked.connect(self.fillInputFileEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.maskSelector.selectClicked.connect(self.fillMaskFileEdit)
        self.extentSelector.newExtentDefined.connect(self.extentChanged)
        self.extentSelector.selectionStarted.connect(self.checkRun)

        self.extentModeRadio.toggled.connect(self.switchClippingMode)
        self.keepResolutionRadio.toggled.connect(self.switchResolutionMode)

    def show_(self):
        self.switchClippingMode()
        self.switchResolutionMode()
        BasePluginWidget.show_(self)

    def onClosing(self):
        self.extentSelector.stop()
        BasePluginWidget.onClosing(self)

    def switchClippingMode(self):
        if self.extentModeRadio.isChecked():
            index = 0
            self.extentSelector.start()
        else:
            self.extentSelector.stop()
            index = 1
        self.modeStackedWidget.setCurrentIndex(index)
        self.checkRun()

    def switchResolutionMode(self):
        if self.keepResolutionRadio.isChecked():
            self.resolutionWidget.hide()
        else:
            self.resolutionWidget.show()

    def checkRun(self):
        if self.extentModeRadio.isChecked():
            enabler = self.extentSelector.isCoordsValid()
        else:
            enabler = not self.maskSelector.filename() == ''
        self.base.enableRun(enabler)

    def extentChanged(self):
        self.activateWindow()
        self.raise_()
        self.checkRun()

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())
        self.maskSelector.setLayers([x for x in Utils.LayerRegistry.instance().getVectorLayers() if x.geometryType() == QGis.Polygon])
        self.checkRun()

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Polygonize"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if inputFile == '':
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if outputFile == '':
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def fillMaskFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        maskFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the mask file"), Utils.FileFilter.allVectorsFilter(), lastUsedFilter)
        if maskFile == '':
            return
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

        self.maskSelector.setFilename(maskFile)
        self.checkRun()

    def getArguments(self):
        if not self.extentModeRadio.isChecked():
            return self.getArgsModeMask()
        return self.getArgsModeExtent()

    def getArgsModeExtent(self):
        self.base.setPluginCommand("gdal_translate")
        inputFn = self.getInputFileName()
        arguments = []
        if self.noDataCheck.isChecked():
            arguments.append("-a_nodata")
            arguments.append(unicode(self.noDataSpin.value()))
        if self.extentModeRadio.isChecked() and self.extentSelector.isCoordsValid():
            rect = self.extentSelector.getExtent()
            if rect is not None and not inputFn == '':
                arguments.append("-projwin")
                arguments.append(unicode(rect.xMinimum()))
                arguments.append(unicode(rect.yMaximum()))
                arguments.append(unicode(rect.xMaximum()))
                arguments.append(unicode(rect.yMinimum()))
        outputFn = self.getOutputFileName()
        if not outputFn == '':
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(inputFn)
        arguments.append(outputFn)
        return arguments

    def getArgsModeMask(self):
        self.base.setPluginCommand("gdalwarp")
        inputFn = self.getInputFileName()
        arguments = []
        if self.noDataCheck.isChecked():
            arguments.append("-dstnodata")
            arguments.append(unicode(self.noDataSpin.value()))
        if self.maskModeRadio.isChecked():
            mask = self.maskSelector.filename()
            if not mask == '' and not inputFn == '':
                arguments.append("-q")
                arguments.append("-cutline")
                arguments.append(mask)
                if Utils.GdalConfig.versionNum() >= 1800:
                    if self.cropToCutlineCheck.isChecked():
                        arguments.append("-crop_to_cutline")
                if self.alphaBandCheck.isChecked():
                    arguments.append("-dstalpha")
                if self.keepResolutionRadio.isChecked():
                    resolution = Utils.getRasterResolution(inputFn)
                    if resolution is not None:
                        arguments.append("-tr")
                        arguments.append(resolution[0])
                        arguments.append(resolution[1])
                else:
                    arguments.append("-tr")
                    arguments.append(unicode(self.xRes.value()))
                    arguments.append(unicode(self.yRes.value()))
        outputFn = self.getOutputFileName()
        if not outputFn == '':
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(inputFn)
        arguments.append(outputFn)
        return arguments

    def getOutputFileName(self):
        return self.outSelector.filename()

    def getInputFileName(self):
        return self.inSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())
