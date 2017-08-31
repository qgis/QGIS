# -*- coding: utf-8 -*-

"""
***************************************************************************
    slope.py
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

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class slope(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    ZEVENBERGEN = 'ZEVENBERGEN'
    AS_PERCENT = 'AS_PERCENT'
    SCALE = 'SCALE'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(
            self.BAND, self.tr('Band number'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(
            self.COMPUTE_EDGES, self.tr('Compute edges'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(
            self.ZEVENBERGEN, self.tr("Use Zevenbergen&Thorne formula (instead of the Horn's one)"),
            defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(
            self.AS_PERCENT, self.tr('Slope expressed as percent (instead of degrees)'),
            defaultValue=False))
        self.addParameter(QgsProcessingParameterNumber(
            self.SCALE, self.tr('Ratio of vertical units to horizontal'),
            type=QgsProcessingParameterNumber.Double,
            minValue=0.0, maxValue=99999999.999999, defaultValue=1.0))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Slope')))

    def name(self):
        return 'slope'

    def displayName(self):
        return self.tr('Slope')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = ['slope']
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        arguments.append(inLayer.source())

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(out)

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBool(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        if self.parameterAsBool(parameters, self.ZEVENBERGEN, context):
            arguments.append('-alg')
            arguments.append('ZevenbergenThorne')

        if self.parameterAsBool(parameters, self.AS_PERCENT, context):
            arguments.append('-p')

        arguments.append('-s')
        arguments.append(str(self.parameterAsDouble(parameters, self.SCALE, context)))

        return ['gdaldem', GdalUtils.escapeAndJoin(arguments)]
