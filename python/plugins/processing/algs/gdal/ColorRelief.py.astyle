# -*- coding: utf-8 -*-

"""
***************************************************************************
    ColorRelief.py
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

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ColorRelief(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    COLOR_TABLE = 'COLOR_TABLE'
    MATCH_MODE = 'MATCH_MODE'
    OUTPUT = 'OUTPUT'

    MATCHING_MODES = ['"0,0,0,0" RGBA', 'Exact color', 'Nearest color']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(
            self.BAND, self.tr('Band number'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'), defaultValue=False))
        self.addParameter(QgsProcessingParameterFile(self.COLOR_TABLE,
                                                     self.tr('Color configuration file'), optional=False))
        self.addParameter(QgsProcessingParameterEnum(self.MATCH_MODE,
                                                     self.tr('Matching mode'), options=self.MATCHING_MODES, defaultValue=0))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Color relief')))

    def name(self):
        return 'colorrelief'

    def displayName(self):
        return self.tr('Color relief')

    def group(self):
        return self.tr('Raster analysis')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = ['color-relief']
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        arguments.append(inLayer.source())
        arguments.append(self.parameterAsFile(parameters, self.COLOR_TABLE, context))
        #filePath = unicode(self.getParameterValue(self.COLOR_TABLE))
        #if filePath is None or filePath == '':
        #    filePath = os.path.join(os.path.dirname(__file__), 'terrain.txt')
        #arguments.append(filePath)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(out)

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBool(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        mode = self.parameterAsEnum(parameters, self.MATCH_MODE, context)
        if mode == 1:
            arguments.append('-exact_color_entry')
        elif mode == 2:
            arguments.append('-nearest_color_entry')

        return ['gdaldem', GdalUtils.escapeAndJoin(arguments)]
