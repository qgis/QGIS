# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrpointsonlines.py
    ---------------------
    Date                 : Janaury 2015
    Copyright            : (C) 2015 by Giovanni Manghi
    Email                : giovanni dot manghi at naturalgis dot pt
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

__author__ = 'Giovanni Manghi'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Giovanni Manghi'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessing,
                       QgsVectorFileWriter,
                       QgsVectorLayer)
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects
from processing.tools.system import isWindows
from processing.tools.vector import ogrConnectionString, ogrLayerName


class Ogr2OgrPointsOnLines(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    DISTANCE = 'DISTANCE'
    GEOMETRY = 'GEOMETRY'
    OPTIONS = 'OPTIONS'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorLine], optional=False))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY,
                                                       self.tr('Geometry column name ("geometry" for Shapefiles, may be different for other formats)'),
                                                       defaultValue='geometry', optional=False))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Distance from line start represented as fraction of line length'), type=QgsProcessingParameterNumber.Double, minValue=0, maxValue=1, defaultValue=0.5))
        self.addParameter(QgsProcessingParameterString(self.OPTIONS,
                                                       self.tr('Additional creation options (see ogr2ogr manual)'),
                                                       defaultValue='', optional=True))

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT_LAYER, self.tr('Points along lines'), QgsProcessing.TypeVectorPoint))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT_LAYER, self.tr("Points along lines"), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'createpointsalonglines'

    def displayName(self):
        return self.tr('Create points along lines')

    def group(self):
        return self.tr('Vector geoprocessing')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsVectorLayer(parameters, self.INPUT_LAYER, context)
        if inLayer is None or inLayer.dataProvider().name() == 'ogr':
            ogrLayer = self.parameterAsCompatibleSourceLayerPath(parameters, self.INPUT_LAYER, context, QgsVectorFileWriter.supportedFormatExtensions(), feedback=feedback)
            if inLayer is None:
                inLayer = ogrLayer
        else:
            ogrLayer = ogrConnectionString(inLayer, context)[1:-1]

        layername = "'" + ogrLayerName(inLayer) + "'"
        distance = str(self.parameterAsDouble(parameters, self.DISTANCE, context))
        geometry = str(self.parameterAsString(parameters, self.GEOMETRY, context))

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT_LAYER, context)

        output = ogrConnectionString(outFile, context)
        options = str(self.parameterAsString(parameters, self.OPTIONS, context))

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)

        arguments.append('-dialect sqlite -sql "SELECT ST_Line_Interpolate_Point(')
        arguments.append(geometry)
        arguments.append(',')
        arguments.append(distance)
        arguments.append('),*')
        arguments.append('FROM')
        arguments.append(layername)
        arguments.append('"')

        if options is not None and len(options.strip()) > 0:
            arguments.append(options)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        return commands

    def commandName(self):
        return "ogr2ogr"
