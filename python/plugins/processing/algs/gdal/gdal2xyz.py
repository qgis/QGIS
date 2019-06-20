# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdal2xyz.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
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
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

from qgis.core import (QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFileDestination
                       )
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows


class gdal2xyz(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    CSV = 'CSV'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.CSV,
                                                        self.tr('Output comma-separated values'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT,
                                                                self.tr('XYZ ASCII file'),
                                                                self.tr('CSV files (*.csv)')))

    def name(self):
        return 'gdal2xyz'

    def displayName(self):
        return self.tr('gdal2xyz')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def commandName(self):
        return 'gdal2xyz'

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagDisplayNameIsLiteral

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []
        arguments.append('-band')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBoolean(parameters, self.CSV, context):
            arguments.append('-csv')

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        arguments.append(raster.source())
        arguments.append(self.parameterAsFileOutput(parameters, self.OUTPUT, context))

        if isWindows():
            commands = ["python3", "-m", self.commandName()]
        else:
            commands = [self.commandName() + '.py']

        commands.append(GdalUtils.escapeAndJoin(arguments))

        return commands
