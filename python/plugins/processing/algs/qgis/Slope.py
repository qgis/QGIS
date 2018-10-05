# -*- coding: utf-8 -*-

"""
***************************************************************************
    Slope.py
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

from qgis.analysis import QgsSlopeFilter
from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Slope(QgisAlgorithm):

    INPUT = 'INPUT'
    Z_FACTOR = 'Z_FACTOR'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'dem.png'))

    def group(self):
        return self.tr('Raster terrain analysis')

    def groupId(self):
        return 'rasterterrainanalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Elevation layer')))
        self.addParameter(QgsProcessingParameterNumber(self.Z_FACTOR,
                                                       self.tr('Z factor'), QgsProcessingParameterNumber.Double,
                                                       1, False, 0.00))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Slope')))

    def name(self):
        return 'slope'

    def displayName(self):
        return self.tr('Slope')

    def processAlgorithm(self, parameters, context, feedback):
        inputFile = self.parameterAsRasterLayer(parameters, self.INPUT, context).source()
        zFactor = self.parameterAsDouble(parameters, self.Z_FACTOR, context)

        outputFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        outputFormat = QgsRasterFileWriter.driverForExtension(os.path.splitext(outputFile)[1])

        slope = QgsSlopeFilter(inputFile, outputFile, outputFormat)
        slope.setZFactor(zFactor)
        slope.processRaster(feedback)

        return {self.OUTPUT: outputFile}
