# -*- coding: utf-8 -*-

"""
***************************************************************************
    hillshade.py
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
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRasterDestination)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class hillshade(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    ZEVENBERGEN = 'ZEVENBERGEN'
    Z_FACTOR = 'Z_FACTOR'
    SCALE = 'SCALE'
    AZIMUTH = 'AZIMUTH'
    ALTITUDE = 'ALTITUDE'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.ZEVENBERGEN,
                                                        self.tr("Use Zevenbergen&Thorne formula (instead of the Horn's one)"), defaultValue=False))
        self.addParameter(QgsProcessingParameterNumber(self.Z_FACTOR,
                                                       self.tr('Z factor (vertical exaggeration)'),
                                                       type=QgsProcessingParameterNumber.Double, minValue=0.0, maxValue=99999999.999999, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.SCALE,
                                                       self.tr('Scale (ratio of vert. units to horiz.)'),
                                                       type=QgsProcessingParameterNumber.Double, minValue=0.0, maxValue=99999999.999999, defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.AZIMUTH,
                                                       self.tr('Azimuth of the light'),
                                                       type=QgsProcessingParameterNumber.Double, minValue=0.0, maxValue=359.9, defaultValue=315.0))
        self.addParameter(QgsProcessingParameterNumber(self.ALTITUDE,
                                                       self.tr('Altitude of the light'),
                                                       type=QgsProcessingParameterNumber.Double, minValue=0.0, maxValue=99999999.999999, defaultValue=45.0))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Hillshade')))

    def name(self):
        return 'hillshade'

    def displayName(self):
        return self.tr('Hillshade')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = ['hillshade']
        arguments.append(self.parameterAsRasterLayer(parameters, self.INPUT, context).source())
        arguments.append(str(self.parameterAsOutputLayer(parameters, self.OUTPUT, context)))

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))
        arguments.append('-z')
        arguments.append(str(self.parameterAsDouble(parameters, self.Z_FACTOR, context)))
        arguments.append('-s')
        arguments.append(str(self.parameterAsDouble(parameters, self.SCALE, context)))
        arguments.append('-az')
        arguments.append(str(self.parameterAsDouble(parameters, self.AZIMUTH, context)))
        arguments.append('-alt')
        arguments.append(str(self.parameterAsDouble(parameters, self.ALTITUDE, context)))

        if self.parameterAsBool(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        if self.parameterAsBool(parameters, self.ZEVENBERGEN, context):
            arguments.append('-alg')
            arguments.append('ZevenbergenThorne')

        return ['gdaldem', GdalUtils.escapeAndJoin(arguments)]
