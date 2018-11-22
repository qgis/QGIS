# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsAlongLines.py
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

__author__ = 'Giovanni Manghi'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Giovanni Manghi'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterDefinition,
                       QgsProcessing)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows


class PointsAlongLines(GdalAlgorithm):

    INPUT = 'INPUT'
    GEOMETRY = 'GEOMETRY'
    DISTANCE = 'DISTANCE'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY,
                                                       self.tr('Geometry column name'),
                                                       defaultValue='geometry'))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Distance from line start represented as fraction of line length'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0,
                                                       maxValue=1,
                                                       defaultValue=0.5))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(options_param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Points along lines'),
                                                                  QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'pointsalonglines'

    def displayName(self):
        return self.tr('Points along lines')

    def group(self):
        return self.tr('Vector geoprocessing')

    def groupId(self):
        return 'vectorgeoprocessing'

    def commandName(self):
        return 'ogr2ogr'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        fields = source.fields()
        ogrLayer, layerName = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        geometry = self.parameterAsString(parameters, self.GEOMETRY, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)

        output, outputFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        other_fields = []
        for f in fields:
            if f.name() == geometry:
                continue
            other_fields.append(f.name())

        if other_fields:
            other_fields = ',*'
        else:
            other_fields = ''

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        sql = "SELECT ST_Line_Interpolate_Point({}, {}) AS {}{} FROM '{}'".format(geometry, distance, geometry, other_fields, layerName)
        arguments.append(sql)

        if options:
            arguments.append(options)

        if outputFormat:
            arguments.append('-f {}'.format(outputFormat))

        return ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]
