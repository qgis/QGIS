# -*- coding: utf-8 -*-

"""
***************************************************************************
    Dissolve.py
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
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class Dissolve(GdalAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    GEOMETRY = 'GEOMETRY'
    EXPLODE_COLLECTIONS = 'EXPLODE_COLLECTIONS'
    KEEP_ATTRIBUTES = 'KEEP_ATTRIBUTES'
    COUNT_FEATURES = 'COUNT_FEATURES'
    COMPUTE_AREA = 'COMPUTE_AREA'
    COMPUTE_STATISTICS = 'COMPUTE_STATISTICS'
    STATISTICS_ATTRIBUTE = 'STATISTICS_ATTRIBUTE'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Dissolve field'),
                                                      None,
                                                      self.INPUT,
                                                      QgsProcessingParameterField.Any, optional=True))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY,
                                                       self.tr('Geometry column name'),
                                                       defaultValue='geometry'))
        params = []
        params.append(QgsProcessingParameterBoolean(self.EXPLODE_COLLECTIONS,
                                                    self.tr('Produce one feature for each geometry in any kind of geometry collection in the source file'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.KEEP_ATTRIBUTES,
                                                    self.tr('Keep input attributes'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.COUNT_FEATURES,
                                                    self.tr('Count dissolved features'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.COMPUTE_AREA,
                                                    self.tr('Compute area and perimeter of dissolved features'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.COMPUTE_STATISTICS,
                                                    self.tr('Compute min/max/sum/mean for attribute'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterField(self.STATISTICS_ATTRIBUTE,
                                                  self.tr('Numeric attribute to calculate statistics on'),
                                                  None,
                                                  self.INPUT,
                                                  QgsProcessingParameterField.Numeric,
                                                  optional=True))
        params.append(QgsProcessingParameterString(self.OPTIONS,
                                                   self.tr('Additional creation options'),
                                                   defaultValue='',
                                                   optional=True))
        for param in params:
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Dissolved')))

    def name(self):
        return 'dissolve'

    def displayName(self):
        return self.tr('Dissolve')

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
        fieldName = self.parameterAsString(parameters, self.FIELD, context)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

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
        arguments.append('-nlt PROMOTE_TO_MULTI')
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        tokens = []
        if self.parameterAsBool(parameters, self.COUNT_FEATURES, context):
            tokens.append("COUNT({}) AS count".format(geometry))

        if self.parameterAsBool(parameters, self.COMPUTE_AREA, context):
            tokens.append("SUM(ST_Area({0})) AS area, ST_Perimeter(ST_Union({0})) AS perimeter".format(geometry))

        statsField = self.parameterAsString(parameters, self.STATISTICS_ATTRIBUTE, context)
        if statsField and self.parameterAsBool(parameters, self.COMPUTE_STATISTICS, context):
            tokens.append("SUM({0}) AS sum, MIN({0}) AS min, MAX({0}) AS max, AVG({0}) AS avg".format(statsField))

        params = ','.join(tokens)
        if params:
            params = ', ' + params

        group_by = ''
        if fieldName:
            group_by = ' GROUP BY {}'.format(fieldName)

        if self.parameterAsBool(parameters, self.KEEP_ATTRIBUTES, context):
            sql = "SELECT ST_Union({}) AS {}{}{} FROM '{}'{}".format(geometry, geometry, other_fields, params, layerName, group_by)
        else:
            sql = "SELECT ST_Union({}) AS {}{}{} FROM '{}'{}".format(geometry, geometry, ', ' + fieldName if fieldName else '',
                                                                     params, layerName, group_by)

        arguments.append(sql)

        if self.parameterAsBool(parameters, self.EXPLODE_COLLECTIONS, context):
            arguments.append('-explodecollections')

        if options:
            arguments.append(options)

        if outputFormat:
            arguments.append('-f {}'.format(outputFormat))

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
