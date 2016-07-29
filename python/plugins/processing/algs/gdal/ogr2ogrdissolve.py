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

from processing.tools.system import isWindows
from processing.tools.vector import ogrConnectionString, ogrLayerName


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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Dissolve polygons')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Geoprocessing')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON], False))
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

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Dissolved')))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = ogrConnectionString(inLayer)[1:-1]
        layername = "'" + ogrLayerName(inLayer) + "'"
        geometry = unicode(self.getParameterValue(self.GEOMETRY))
        field = unicode(self.getParameterValue(self.FIELD))
        statsatt = unicode(self.getParameterValue(self.STATSATT))
        stats = self.getParameterValue(self.STATS)
        area = self.getParameterValue(self.AREA)
        multi = self.getParameterValue(self.MULTI)
        count = self.getParameterValue(self.COUNT)
        fields = self.getParameterValue(self.FIELDS)
        querystart = '-dialect sqlite -sql "SELECT ST_Union(' + geometry + ')'
        queryend = ' FROM ' + layername + ' GROUP BY ' + field + '"'
        if fields:
            queryfields = ",*"
        else:
            queryfields = "," + field
        if count:
            querycount = ", COUNT(" + geometry + ") AS count"
        else:
            querycount = ""
        if stats:
            querystats = ", SUM(" + statsatt + ") AS sum_diss, MIN(" + statsatt + ") AS min_diss, MAX(" + statsatt + ") AS max_diss, AVG(" + statsatt + ") AS avg_diss"
        else:
            querystats = ""
        if area:
            queryarea = ", SUM(ST_area(" + geometry + ")) AS area_diss, ST_perimeter(ST_union(" + geometry + ")) AS peri_diss"
        else:
            queryarea = ""

        query = querystart + queryfields + querycount + querystats + queryarea + queryend
        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(ogrLayerName(inLayer))
        arguments.append(query)

        if not multi:
            arguments.append('-explodecollections')

        if len(options) > 0:
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
