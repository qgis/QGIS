# -*- coding: utf-8 -*-

"""
***************************************************************************
    ReliefAuto.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.analysis import QgsRelief

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputTable
from processing.tools import raster

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ReliefAuto(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    Z_FACTOR = 'Z_FACTOR'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    FREQUENCY_DISTRIBUTION = 'FREQUENCY_DISTRIBUTION'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'dem.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Relief (automatic colors)')
        self.group, self.i18n_group = self.trAlgorithm('Raster terrain analysis')

        self.addParameter(ParameterRaster(self.INPUT_LAYER,
                                          self.tr('Elevation layer')))
        self.addParameter(ParameterNumber(self.Z_FACTOR,
                                          self.tr('Z factor'), 1.0, 999999.99, 1.0))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Relief')))
        self.addOutput(OutputTable(self.FREQUENCY_DISTRIBUTION,
                                   self.tr('Frequency distribution')))

    def processAlgorithm(self, progress):
        inputFile = self.getParameterValue(self.INPUT_LAYER)
        zFactor = self.getParameterValue(self.Z_FACTOR)
        outputFile = self.getOutputValue(self.OUTPUT_LAYER)
        frequencyDistribution = self.getOutputValue(self.FREQUENCY_DISTRIBUTION)

        outputFormat = raster.formatShortNameFromFileName(outputFile)

        relief = QgsRelief(inputFile, outputFile, outputFormat)
        colors = relief.calculateOptimizedReliefClasses()
        relief.setReliefColors(colors)
        relief.setZFactor(zFactor)
        relief.exportFrequencyDistributionToCsv(frequencyDistribution)
        relief.processRaster(None)
