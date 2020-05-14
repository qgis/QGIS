# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdal2tiles.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingParameterRasterLayer)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ValidateCog(GdalAlgorithm):

    INPUT = 'INPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))

    def name(self):
        return 'validate_cog'

    def displayName(self):
        return self.tr('Validate COG (Cloud Optimized Geotiff)')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'tiles.png'))

    def commandName(self):
        return os.path.join(os.path.dirname(__file__), 'helpers', 'validate_cloud_optimized_geotiff')

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagDisplayNameIsLiteral

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []

        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        arguments.append(inLayer.source())

        if isWindows():
            commands = ["python3", self.commandName() + '.py']
        else:
            commands = [self.commandName() + '.py']

        commands.append(GdalUtils.escapeAndJoin(arguments))

        return commands
