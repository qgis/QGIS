# -*- coding: utf-8 -*-

"""
***************************************************************************
    tri.py
    ---------------------
    Date                 : October 2013
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
from builtins import str

__author__ = 'Alexander Bruy'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class tri(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'), defaultValue=False))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Terrain Ruggedness Index')))

    def name(self):
        return 'triterrainruggednessindex'

    def displayName(self):
        return self.tr('TRI (Terrain Ruggedness Index)')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = ['TRI']
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        arguments.append(inLayer.source())
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(out)

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBool(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        return ['gdaldem', GdalUtils.escapeAndJoin(arguments)]
