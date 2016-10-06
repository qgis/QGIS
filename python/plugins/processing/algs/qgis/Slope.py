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

from qgis.analysis import QgsSlopeFilter

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools import raster


class Slope(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    Z_FACTOR = 'Z_FACTOR'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Slope')
        self.group, self.i18n_group = self.trAlgorithm('Raster terrain analysis')

        self.addParameter(ParameterRaster(self.INPUT_LAYER,
                                          self.tr('Elevation layer')))
        self.addParameter(ParameterNumber(self.Z_FACTOR,
                                          self.tr('Z factor'), 1.0, 999999.99, 1.0))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Slope')))

    def processAlgorithm(self, progress):
        inputFile = self.getParameterValue(self.INPUT_LAYER)
        zFactor = self.getParameterValue(self.Z_FACTOR)
        outputFile = self.getOutputValue(self.OUTPUT_LAYER)

        outputFormat = raster.formatShortNameFromFileName(outputFile)

        slope = QgsSlopeFilter(inputFile, outputFile, outputFormat)
        slope.setZFactor(zFactor)
        slope.processRaster(None)
