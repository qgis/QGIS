# -*- coding: utf-8 -*-

"""
***************************************************************************
    doRasterize.py
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

from qgis.PyQt.QtCore import QFileInfo, QTextCodec
from qgis.PyQt.QtWidgets import QWidget, QMessageBox, QErrorMessage

from .ui_widgetRasterize import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_rasterize")

        self.outSelector.setType(self.outSelector.FILE)

        # set the default QSpinBoxes and QProgressBar value
        self.widthSpin.setValue(3000)
        self.heightSpin.setValue(3000)
        self.horizresSpin.setValue(1)
        self.vertresSpin.setValue(1)

        self.lastEncoding = Utils.getLastUsedEncoding()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.attributeComboBox, "currentIndexChanged"),
            ([self.widthSpin, self.heightSpin], "valueChanged"),
            ([self.horizresSpin, self.vertresSpin], "valueChanged")
        ])

        self.inSelector.selectClicked.connect(self.fillInputFileEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.inSelector.layerChanged.connect(self.fillFieldsCombo)
        self.radioSetSize.toggled.connect(self.someValueChanged)
        self.radioSetResolution.toggled.connect(self.someValueChanged)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getVectorLayers())

    def fillFieldsCombo(self):
        if self.inSelector.layer() is None:
            return
        self.lastEncoding = self.inSelector.layer().dataProvider().encoding()
        self.loadFields(self.getInputFileName())

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        inputFile, encoding = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Rasterize"), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)
        self.lastEncoding = encoding

        self.loadFields(inputFile)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()

        # rasterize supports output file creation for GDAL 1.8
        gdalVersion = Utils.GdalConfig.versionNum()
        if gdalVersion >= 1800:
            fileDialogFunc = Utils.FileDialog.getSaveFileName
            filters = Utils.FileFilter.saveRastersFilter()
        else:
            fileDialogFunc = Utils.FileDialog.getOpenFileName
            filters = Utils.FileFilter.allRastersFilter()

        outputFile = fileDialogFunc(self, self.tr("Select the raster file to save the results to"), filters, lastUsedFilter)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.outSelector.setFilename(outputFile)

        # required either -ts or -tr to create the output file
        if gdalVersion >= 1800:
            if not QFileInfo(outputFile).exists():
                QMessageBox.information(self, self.tr("Output size or resolution required"), self.tr("The output file doesn't exist. You must set up the output size or resolution to create it."))
                self.radioSetSize.setChecked(True)

    def getArguments(self):
        arguments = []
        if self.attributeComboBox.currentIndex() >= 0:
            arguments.append("-a")
            arguments.append(self.attributeComboBox.currentText())
        if self.radioSetSize.isChecked():
            arguments.append("-ts")
            arguments.append(self.widthSpin.value())
            arguments.append(self.heightSpin.value())
        if self.radioSetResolution.isChecked():
            arguments.append("-tr")
            arguments.append(self.horizresSpin.value())
            arguments.append(self.vertresSpin.value())
        inputFn = self.getInputFileName()
        if inputFn:
            arguments.append("-l")
            arguments.append(QFileInfo(inputFn).baseName())
        arguments.append(inputFn)
        arguments.append(self.getOutputFileName())
        return arguments

    def getInputFileName(self):
        return self.inSelector.filename()

    def getOutputFileName(self):
        return self.outSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())

    def loadFields(self, vectorFile):
        self.attributeComboBox.clear()

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
            self.attributeComboBox.addItem(ncodec.toUnicode(name))
