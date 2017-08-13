# -*- coding: utf-8 -*-

"""
***************************************************************************
    offsetcurve.py
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
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects
from processing.tools.system import isWindows


class OffsetCurve(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    GEOMETRY = 'GEOMETRY'
    RADIUS = 'RADIUS'
    OPTIONS = 'OPTIONS'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE], False))
        self.addParameter(ParameterString(self.GEOMETRY,
                                          self.tr('Geometry column name ("geometry" for Shapefiles, may be different for other formats)'),
                                          'geometry', optional=False))
        self.addParameter(ParameterNumber(self.RADIUS,
                                          self.tr('Offset distance (positive value for left-sided and negative - for right-sided)'),
                                          -99999999.999999, 99999999.999999, 1000.0,
                                          optional=False))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options (see ogr2ogr manual)'),
                                          '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Offset line')))

    def name(self):
        return 'offsetlinesforlines'

    def displayName(self):
        return self.tr('Offset lines for lines')

    def group(self):
        return self.tr('Vector geoprocessing')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        geometry = self.getParameterValue(self.GEOMETRY)
        distance = self.getParameterValue(self.RADIUS)
        options = self.getParameterValue(self.OPTIONS)

        ogrLayer = GdalUtils.ogrConnectionString(inLayer, context)[1:-1]
        layername = "'" + GdalUtils.ogrLayerName(inLayer) + "'"

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value
        output = GdalUtils.ogrConnectionString(outFile, context)

        layername = GdalUtils.ogrLayerName(inLayer)

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        sql = "SELECT ST_OffsetCurve({}, {}), * FROM '{}'".format(geometry, distance, layername)

        arguments.append(sql)

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
