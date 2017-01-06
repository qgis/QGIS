# -*- coding: utf-8 -*-

"""
***************************************************************************
    Hillshade.py
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

from qgis.analysis import QgsHillshadeFilter

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools import raster

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Hillshade(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    Z_FACTOR = 'Z_FACTOR'
    AZIMUTH = 'AZIMUTH'
    V_ANGLE = 'V_ANGLE'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'dem.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Hillshade')
        self.group, self.i18n_group = self.trAlgorithm('Raster terrain analysis')

        self.addParameter(ParameterRaster(self.INPUT_LAYER,
                                          self.tr('Elevation layer')))
        self.addParameter(ParameterNumber(self.Z_FACTOR,
                                          self.tr('Z factor'), 1.0, 999999.99, 1.0))
        self.addParameter(ParameterNumber(self.AZIMUTH,
                                          self.tr('Azimuth (horizontal angle)'), 0.00, 360.00, 300.00))
        self.addParameter(ParameterNumber(self.V_ANGLE,
                                          self.tr('Vertical angle'), 1.00, 90.00, 40.00))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Hillshade')))

    def processAlgorithm(self, feedback):
        inputFile = self.getParameterValue(self.INPUT_LAYER)
        zFactor = self.getParameterValue(self.Z_FACTOR)
        azimuth = self.getParameterValue(self.AZIMUTH)
        vAngle = self.getParameterValue(self.V_ANGLE)
        outputFile = self.getOutputValue(self.OUTPUT_LAYER)

        outputFormat = raster.formatShortNameFromFileName(outputFile)

        hillshade = QgsHillshadeFilter(inputFile, outputFile, outputFormat, azimuth, vAngle)
        hillshade.setZFactor(zFactor)
        hillshade.processRaster(None)
