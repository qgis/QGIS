# -*- coding: utf-8 -*-

"""
***************************************************************************
    doPolygonize.py
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

from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtWidgets import QWidget

from .ui_widgetPolygonize import Ui_GdalToolsWidget as Ui_Widget
from .widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
from . import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface
        self.resolutions = ("highest", "average", "lowest")

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_polygonize.py")

        self.outSelector.setType(self.outSelector.FILE)
        self.outputFormat = Utils.fillVectorOutputFormat()

        self.setParamsStatus([
            (self.inSelector, "filenameChanged"),
            (self.outSelector, "filenameChanged"),
            (self.maskSelector, "filenameChanged", self.maskCheck),
            (self.fieldEdit, "textChanged", self.fieldCheck)
        ])

        self.inSelector.selectClicked.connect(self.fillInputFileEdit)
        self.outSelector.selectClicked.connect(self.fillOutputFileEdit)
        self.maskSelector.selectClicked.connect(self.fillMaskFileEdit)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())
        self.maskSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Polygonize"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not inputFile:
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.inSelector.setFilename(inputFile)

    def fillOutputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedVectorFilter()
        outputFile, encoding = Utils.FileDialog.getSaveFileName(self, self.tr("Select where to save the Polygonize output"), Utils.FileFilter.allVectorsFilter(), lastUsedFilter, True)
        if not outputFile:
            return
        Utils.FileFilter.setLastUsedVectorFilter(lastUsedFilter)

        self.outputFormat = Utils.fillVectorOutputFormat(lastUsedFilter, outputFile)
        self.outSelector.setFilename(outputFile)
        self.lastEncoding = encoding

    def fillMaskFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        maskFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Polygonize"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
        if not maskFile():
            return
        Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

        self.maskSelector.setFilename(maskFile)

    def getArguments(self):
        arguments = []
        arguments.append(self.getInputFileName())
        outputFn = self.getOutputFileName()
        maskFn = self.getMaskFileName()
        if self.maskCheck.isChecked() and maskFn:
            arguments.append("-mask")
            arguments.append(maskFn)
        if outputFn:
            arguments.append("-f")
            arguments.append(self.outputFormat)
        arguments.append(outputFn)
        if outputFn:
            arguments.append(QFileInfo(outputFn).baseName())
        if (self.fieldCheck.isChecked() and self.fieldEdit.text()):
            arguments.append(self.fieldEdit.text())
        return arguments

    def getOutputFileName(self):
        return self.outSelector.filename()

    def getInputFileName(self):
        return self.inSelector.filename()

    def getMaskFileName(self):
        return self.maskSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        vl = self.iface.addVectorLayer(fileInfo.filePath(), fileInfo.baseName(), "ogr")
        if vl is not None and vl.isValid():
            if hasattr(self, 'lastEncoding'):
                vl.setProviderEncoding(self.lastEncoding)
