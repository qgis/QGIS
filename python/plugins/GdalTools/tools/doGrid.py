# -*- coding: utf-8 -*-

"""
***************************************************************************
    doGrid.py
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
from builtins import str

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QFileInfo, QTextCodec
from qgis.PyQt.QtWidgets import QWidget, QErrorMessage

from .ui_widgetGrid import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.canvas = self.iface.mapCanvas()
        self.algorithm = ('invdist', 'average', 'nearest', 'datametrics')
        self.datametrics = ('minimum', 'maximum', 'range')

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_grid")

        self.outSelector.setType(self.outSelector.FILE)
        self.extentSelector.setCanvas(self.canvas)
        #self.extentSelector.stop()

        # set the default QSpinBoxes and QProgressBar value
        self.widthSpin.setValue(3000)
        self.heightSpin.setValue(3000)
        self.invdistPowerSpin.setValue(2.0)

        self.outputFormat = Utils.fillRasterOutputFormat()
        self.lastEncoding = Utils.getLastUsedEncoding()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.zfieldCombo, "currentIndexChanged", self.zfieldCheck),
            (self.algorithmCombo, "currentIndexChanged", self.algorithmCheck),
            (self.stackedWidget, None, self.algorithmCheck),
            ([self.invdistPowerSpin, self.invdistSmothingSpin, self.invdistRadius1Spin, self.invdistRadius2Spin, self.invdistAngleSpin, self.invdistNoDataSpin], "valueChanged"),
            ([self.invdistMaxPointsSpin, self.invdistMinPointsSpin], "valueChanged"),
            ([self.averageRadius1Spin, self.averageRadius2Spin, self.averageAngleSpin, self.averageNoDataSpin], "valueChanged"),
            (self.averageMinPointsSpin, "valueChanged"),
            ([self.nearestRadius1Spin, self.nearestRadius2Spin, self.nearestAngleSpin, self.nearestNoDataSpin], "valueChanged"),
            (self.datametricsCombo, "currentIndexChanged"),
            ([self.datametricsRadius1Spin, self.datametricsRadius2Spin, self.datametricsAngleSpin, self.datametricsNoDataSpin], "valueChanged"),
            (self.datametricsMinPointsSpin, "valueChanged"),
            (self.extentSelector, ["selectionStarted", "newExtentDefined"], self.extentGroup),
            ([self.widthSpin, self.heightSpin], "valueChanged", self.resizeGroupBox)
        ])

        self.inSelector.selectClicked.connect(self.fillInputFileEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.inSelector.layerChanged.connect(self.fillFieldsCombo)
        self.extentGroup.toggled.connect(self.onExtentCheckedChanged)

    def onClosing(self):
        self.extentSelector.stop()
        BasePluginWidget.onClosing(self)

    def onExtentCheckedChanged(self, enabled):
        self.extentSelector.start() if enabled else self.extentSelector.stop()

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getVectorLayers())

    def fillFieldsCombo(self):
        if self.inSelector.layer() is None:
            return

        self.lastEncoding = self.inSelector.layer().dataProvider().encoding()
        self.loadFields(self.getInputFileName())

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        inputFile, encoding = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Grid"), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)
        self.lastEncoding = encoding

        self.loadFields(inputFile)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        outputFile = Utils.FileDialog.getSaveFileName(self, self.tr("Select the raster file to save the results to"), Utils.FileFilter.saveRastersFilter(), lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)

    def getArguments(self):
        arguments = []
        if self.zfieldCheck.isChecked() and self.zfieldCombo.currentIndex() >= 0:
            arguments.append("-zfield")
            arguments.append(self.zfieldCombo.currentText())
        inputFn = self.getInputFileName()
        if inputFn:
            arguments.append("-l")
            arguments.append(QFileInfo(inputFn).baseName())
        if self.extentGroup.isChecked():
            rect = self.extentSelector.getExtent()
            if rect is not None:
                arguments.append("-txe")
                arguments.append(str(rect.xMinimum()))
                arguments.append(str(rect.xMaximum()))
                arguments.append("-tye")
                arguments.append(str(rect.yMaximum()))
                arguments.append(str(rect.yMinimum()))
        if self.algorithmCheck.isChecked() and self.algorithmCombo.currentIndex() >= 0:
            arguments.append("-a")
            arguments.append(self.algorithmArguments(self.algorithmCombo.currentIndex()))
        if self.resizeGroupBox.isChecked():
            arguments.append("-outsize")
            arguments.append(str(self.widthSpin.value()))
            arguments.append(str(self.heightSpin.value()))
        outputFn = self.getOutputFileName()
        if outputFn:
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(inputFn)
        arguments.append(outputFn)
        return arguments

    def getInputFileName(self):
        return self.inSelector.filename()

    def getOutputFileName(self):
        return self.outSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())

    def algorithmArguments(self, index):
        algorithm = self.algorithm[index]
        arguments = []
        if algorithm == "invdist":
            arguments.append(algorithm)
            arguments.append("power=" + str(self.invdistPowerSpin.value()))
            arguments.append("smothing=" + str(self.invdistSmothingSpin.value()))
            arguments.append("radius1=" + str(self.invdistRadius1Spin.value()))
            arguments.append("radius2=" + str(self.invdistRadius2Spin.value()))
            arguments.append("angle=" + str(self.invdistAngleSpin.value()))
            arguments.append("max_points=" + str(self.invdistMaxPointsSpin.value()))
            arguments.append("min_points=" + str(self.invdistMinPointsSpin.value()))
            arguments.append("nodata=" + str(self.invdistNoDataSpin.value()))
        elif algorithm == "average":
            arguments.append(algorithm)
            arguments.append("radius1=" + str(self.averageRadius1Spin.value()))
            arguments.append("radius2=" + str(self.averageRadius2Spin.value()))
            arguments.append("angle=" + str(self.averageAngleSpin.value()))
            arguments.append("min_points=" + str(self.averageMinPointsSpin.value()))
            arguments.append("nodata=" + str(self.averageNoDataSpin.value()))
        elif algorithm == "nearest":
            arguments.append(algorithm)
            arguments.append("radius1=" + str(self.nearestRadius1Spin.value()))
            arguments.append("radius2=" + str(self.nearestRadius2Spin.value()))
            arguments.append("angle=" + str(self.nearestAngleSpin.value()))
            arguments.append("nodata=" + str(self.nearestNoDataSpin.value()))
        else:
            arguments.append(self.datametrics[self.datametricsCombo.currentIndex()])
            arguments.append("radius1=" + str(self.datametricsRadius1Spin.value()))
            arguments.append("radius2=" + str(self.datametricsRadius2Spin.value()))
            arguments.append("angle=" + str(self.datametricsAngleSpin.value()))
            arguments.append("min_points=" + str(self.datametricsMinPointsSpin.value()))
            arguments.append("nodata=" + str(self.datametricsNoDataSpin.value()))
        return ':'.join(arguments)

    def loadFields(self, vectorFile=''):
        self.zfieldCombo.clear()

        if not vectorFile:
            return

        try:
            (fields, names) = Utils.getVectorFields(vectorFile)
        except Utils.UnsupportedOGRFormat as e:
            QErrorMessage(self).showMessage(e.args[0])
            self.inSelector.setLayer(None)
            return

        ncodec = QTextCodec.codecForName(self.lastEncoding)
        for name in names:
            self.zfieldCombo.addItem(ncodec.toUnicode(name))
