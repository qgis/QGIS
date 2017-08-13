# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrdissolve.py
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

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects
from processing.tools.system import isWindows


class Ogr2OgrDissolve(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    GEOMETRY = 'GEOMETRY'
    FIELD = 'FIELD'
    MULTI = 'MULTI'
    COUNT = 'COUNT'
    STATS = 'STATS'
    STATSATT = 'STATSATT'
    AREA = 'AREA'
    FIELDS = 'FIELDS'
    OPTIONS = 'OPTIONS'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterString(self.GEOMETRY,
                                          self.tr('Geometry column name ("geometry" for Shapefiles, may be different for other formats)'),
                                          'geometry', optional=False))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Dissolve field'), self.INPUT_LAYER))
        self.addParameter(ParameterBoolean(self.MULTI,
                                           self.tr('Output as multipart geometries'), True))
        self.addParameter(ParameterBoolean(self.FIELDS,
                                           self.tr('Keep input attributes'), False))
        self.addParameter(ParameterBoolean(self.COUNT,
                                           self.tr('Count dissolved features'), False))
        self.addParameter(ParameterBoolean(self.AREA,
                                           self.tr('Compute area and perimeter of dissolved features'), False))
        self.addParameter(ParameterBoolean(self.STATS,
                                           self.tr('Compute min/max/sum/mean for the following numeric attribute'), False))
        self.addParameter(ParameterTableField(self.STATSATT,
                                              self.tr('Numeric attribute to compute dissolved features stats'), self.INPUT_LAYER, optional=True))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options (see ogr2ogr manual)'),
                                          '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Dissolved'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def name(self):
        return 'dissolvepolygons'

    def displayName(self):
        return self.tr('Dissolve polygons')

    def group(self):
        return self.tr('Vector geoprocessing')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        geometry = self.getParameterValue(self.GEOMETRY)
        field = self.getParameterValue(self.FIELD)
        multi = self.getParameterValue(self.MULTI)
        fields = self.getParameterValue(self.FIELDS)
        count = self.getParameterValue(self.COUNT)
        area = self.getParameterValue(self.AREA)
        stats = self.getParameterValue(self.STATS)
        statsatt = self.getParameterValue(self.STATSATT)
        options = self.getParameterValue(self.OPTIONS)

        ogrLayer = GdalUtils.ogrConnectionString(inLayer, context)[1:-1]
        layername = GdalUtils.ogrLayerName(inLayer)

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = GdalUtils.ogrConnectionString(outFile, context)

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        sql = "SELECT ST_Union({})".format(geometry)

        sqlOpts = ''
        if fields:
            sqlOpts += ',*'
        else:
            sqlOpts += ',{}'.format(field)

        if count:
            sqlOpts += ", COUNT({}) AS count".format(geometry)

        if stats:
            sqlOpts += ", SUM({0}) AS sum_diss, MIN({0}) AS min_diss, MAX({0}) AS max_diss, AVG({0}) AS avg_diss".format(statsatt)

        if area:
            sqlOpts += ", SUM(ST_Area({0})) AS area_diss, ST_Perimeter(ST_Union({0})) AS peri_diss".format(geometry)

        sql = '{}{} FROM {} GROUP BY {}'.format(sql, sqlOpts, layername, field)

        arguments.append(sql)

        if not multi:
            arguments.append('-explodecollections')

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
        return 'ogr2ogr'
