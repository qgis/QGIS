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

__author__ = 'Giovanni Manghi'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Giovanni Manghi'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector

from processing.tools.system import isWindows

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrPointsOnLines(OgrAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    DISTANCE = 'DISTANCE'
    GEOMETRY = 'GEOMETRY'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Create points along lines')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Geoprocessing')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_LINE], False))
        self.addParameter(ParameterString(self.GEOMETRY,
            self.tr('Geometry column name ("geometry" for Shapefiles, may be different for other formats)'),
            'geometry', optional=False))
        self.addParameter(ParameterNumber(self.DISTANCE,
            self.tr('Distance from line start represented as fraction of line length'), 0, 1, 0.5))
        self.addParameter(ParameterString(self.OPTIONS,
            self.tr('Additional creation options (see ogr2ogr manual)'),
            '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Points along lines')))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)[1:-1]
        layername = "'" + self.ogrLayerName(inLayer) + "'"
        distance = unicode(self.getParameterValue(self.DISTANCE))
        geometry = unicode(self.getParameterValue(self.GEOMETRY))

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = self.ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(self.ogrLayerName(inLayer))

        arguments.append('-dialect sqlite -sql "SELECT ST_Line_Interpolate_Point(')
        arguments.append(geometry)
        arguments.append(',')
        arguments.append(distance)
        arguments.append('),*')
        arguments.append('FROM')
        arguments.append(layername)
        arguments.append('"')

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