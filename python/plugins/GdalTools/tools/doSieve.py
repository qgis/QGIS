# -*- coding: utf-8 -*-

"""
***************************************************************************
    doSieve.py
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

from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QWidget

from ui_widgetSieve import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils


class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

    def __init__(self, iface):
        QWidget.__init__(self)
        self.iface = iface

        self.setupUi(self)
        BasePluginWidget.__init__(self, self.iface, "gdal_sieve.py")

        self.outSelector.setType(self.outSelector.FILE)
        self.outputFormat = Utils.fillRasterOutputFormat()

        self.setParamsStatus([
            (self.inSelector, SIGNAL("filenameChanged()")),
            (self.outSelector, SIGNAL("filenameChanged()")),
            (self.thresholdSpin, SIGNAL("valueChanged(int)"), self.thresholdCheck),
            (self.connectionsCombo, SIGNAL("currentIndexChanged(int)"), self.connectionsCheck)
        ])

        self.connect(self.inSelector, SIGNAL("selectClicked()"), self.fillInputFileEdit)
        self.connect(self.outSelector, SIGNAL("selectClicked()"), self.fillOutputFileEdit)

    def onLayersChanged(self):
        self.inSelector.setLayers(Utils.LayerRegistry.instance().getRasterLayers())

    def fillInputFileEdit(self):
        lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
        inputFile = Utils.FileDialog.getOpenFileName(self, self.tr("Select the input file for Sieve"), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
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

    def getArguments(self):
        arguments = []
        if self.thresholdCheck.isChecked():
            arguments.append("-st")
            arguments.append(unicode(self.thresholdSpin.value()))
        if self.connectionsCheck.isChecked() and self.connectionsCombo.currentIndex() >= 0:
            arguments.append("-" + self.connectionsCombo.currentText())
        outputFn = self.getOutputFileName()
        if outputFn:
            arguments.append("-of")
            arguments.append(self.outputFormat)
        arguments.append(self.getInputFileName())
        arguments.append(outputFn)
        return arguments

    def getOutputFileName(self):
        return self.outSelector.filename()

    def getInputFileName(self):
        return self.inSelector.filename()

    def addLayerIntoCanvas(self, fileInfo):
        self.iface.addRasterLayer(fileInfo.filePath())
