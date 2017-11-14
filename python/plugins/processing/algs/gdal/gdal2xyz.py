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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingOutputFile)
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
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.CSV,
                                                        self.tr('Output comma-separated values'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT,
                                                                self.tr('XYZ ASCII file'),
                                                                self.tr('CSV files (*.csv)')))
        self.addOutput(QgsProcessingOutputFile(self.OUTPUT, self.tr('XYZ ASCII file')))

    def name(self):
        return 'gdal2xyz'

    def displayName(self):
        return self.tr('gdal2xyz')

    def group(self):
        return self.tr('Raster conversion')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        arguments = []
        arguments.append('-band')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBool(parameters, self.CSV, context):
            arguments.append('-csv')

        arguments.append(self.parameterAsRasterLayer(parameters, self.INPUT, context).source())
        arguments.append(self.parameterAsFileOutput(parameters, self.OUTPUT, context))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal2xyz.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal2xyz.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
