# -*- coding: utf-8 -*-

"""
***************************************************************************
    OneSideBuffer.py
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

from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class OneSideBuffer(GdalAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    BUFFER_SIDE = 'BUFFER_SIDE'
    GEOMETRY = 'GEOMETRY'
    DISTANCE = 'DISTANCE'
    DISSOLVE = 'DISSOLVE'
    EXPLODE_COLLECTIONS = 'EXPLODE_COLLECTIONS'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.bufferSides = [self.tr('Right'), self.tr('Left')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY,
                                                       self.tr('Geometry column name'),
                                                       defaultValue='geometry'))
        self.addParameter(QgsProcessingParameterDistance(self.DISTANCE,
                                                         self.tr('Buffer distance'),
                                                         defaultValue=10,
                                                         parentParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterEnum(self.BUFFER_SIDE,
                                                     self.tr('Buffer side'),
                                                     options=self.bufferSides,
                                                     allowMultiple=False,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Dissolve by attribute'),
                                                      None,
                                                      self.INPUT,
                                                      QgsProcessingParameterField.Any,
                                                      optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.DISSOLVE,
                                                        self.tr('Dissolve all results'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.EXPLODE_COLLECTIONS,
                                                        self.tr('Produce one feature for each geometry in any kind of geometry collection in the source file'),
                                                        defaultValue=False))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(options_param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('One-sided buffer'),
                                                                  QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'onesidebuffer'

    def displayName(self):
        return self.tr('One side buffer')

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
        geometry = self.parameterAsString(parameters, self.GEOMETRY, context)
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        side = self.parameterAsEnum(parameters, self.BUFFER_SIDE, context)
        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        dissolve = self.parameterAsBool(parameters, self.DISSOLVE, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

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

        if dissolve or fieldName:
            sql = "SELECT ST_Union(ST_SingleSidedBuffer({}, {}, {})) AS {}{} FROM '{}'".format(geometry, distance, side, geometry, other_fields, layerName)
        else:
            sql = "SELECT ST_SingleSidedBuffer({}, {}, {}) AS {}{} FROM '{}'".format(geometry, distance, side, geometry, other_fields, layerName)

        if fieldName:
            sql = '"{} GROUP BY {}"'.format(sql, fieldName)

        arguments.append(sql)

        if self.parameterAsBool(parameters, self.EXPLODE_COLLECTIONS, context):
            arguments.append('-explodecollections')

        if options:
            arguments.append(options)

        if outputFormat:
            arguments.append('-f {}'.format(outputFormat))

        return ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]
