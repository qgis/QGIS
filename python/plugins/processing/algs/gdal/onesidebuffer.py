# -*- coding: utf-8 -*-

"""
***************************************************************************
    onesidebuffer.py
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
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects
from processing.tools.system import isWindows
from processing.tools.vector import ogrConnectionString, ogrLayerName


class OneSideBuffer(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    GEOMETRY = 'GEOMETRY'
    RADIUS = 'RADIUS'
    LEFTRIGHT = 'LEFTRIGHT'
    LEFTRIGHTLIST = ['Right', 'Left']
    DISSOLVEALL = 'DISSOLVEALL'
    FIELD = 'FIELD'
    MULTI = 'MULTI'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Single sided buffer for lines')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Geoprocessing')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE], False))
        self.addParameter(ParameterString(self.GEOMETRY,
                                          self.tr('Geometry column name ("geometry" for Shapefiles, may be different for other formats)'),
                                          'geometry', optional=False))
        self.addParameter(ParameterString(self.RADIUS,
                                          self.tr('Buffer distance'), '1000', optional=False))
        self.addParameter(ParameterSelection(self.LEFTRIGHT,
                                             self.tr('Buffer side'), self.LEFTRIGHTLIST, 0))
        self.addParameter(ParameterBoolean(self.DISSOLVEALL,
                                           self.tr('Dissolve all results'), False))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Dissolve by attribute'), self.INPUT_LAYER, optional=True))
        self.addParameter(ParameterBoolean(self.MULTI,
                                           self.tr('Output as singlepart geometries (only used when dissolving by attribute)'), False))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options (see ogr2ogr manual)'),
                                          '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Single sided buffer')))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        geometry = self.getParameterValue(self.GEOMETRY)
        distance = self.getParameterValue(self.RADIUS)
        leftright = self.getParameterValue(self.LEFTRIGHT)
        dissolveall = self.getParameterValue(self.DISSOLVEALL)
        field = self.getParameterValue(self.FIELD)
        multi = self.getParameterValue(self.MULTI)
        options = self.getParameterValue(self.OPTIONS)

        ogrLayer = ogrConnectionString(inLayer)[1:-1]
        layername = ogrLayerName(inLayer)

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value
        output = ogrConnectionString(outFile)

        layername = ogrLayerName(inLayer)

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(layername)
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        if dissolveall or field is not None:
            sql = "SELECT ST_Union(ST_SingleSidedBuffer({}, {}, {})), * FROM '{}'".format(geometry, distance, leftright, layername)
        else:
            sql = "SELECT ST_SingleSidedBuffer({},{},{}), * FROM '{}'".format(geometry, distance, leftright, layername)

        if field is not None:
            sql = '"{} GROUP BY {}"'.format(sql, field)

        arguments.append(sql)

        if field is not None and multi:
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
        return 'ogr2ogr'
