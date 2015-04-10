# -*- coding: utf-8 -*-

"""
***************************************************************************
    proximity.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils


class proximity(GdalAlgorithm):

    INPUT = 'INPUT'
    VALUES = 'VALUES'
    UNITS = 'UNITS'
    MAX_DIST = 'MAX_DIST'
    NODATA = 'NODATA'
    BUF_VAL = 'BUF_VAL'
    OUTPUT = 'OUTPUT'
    RTYPE = 'RTYPE'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    DISTUNITS = ['GEO', 'PIXEL']

    def commandLineName(self):
        return "gdalogr:proximity"

    def defineCharacteristics(self):
        self.name = 'Proximity (raster distance)'
        self.group = '[GDAL] Analysis'
        self.addParameter(ParameterRaster(self.INPUT,
            self.tr('Input layer'), False))
        self.addParameter(ParameterString(self.VALUES,
            self.tr('Values'), ''))
        self.addParameter(ParameterSelection(self.UNITS,
            self.tr('Distance units'), self.DISTUNITS, 0))
        self.addParameter(ParameterNumber(self.MAX_DIST,
            self.tr('Max distance (negative value to ignore)'), -1, 9999, -1))
        self.addParameter(ParameterNumber(self.NODATA,
            self.tr('Nodata (negative value to ignore)'), -1, 9999, -1))
        self.addParameter(ParameterNumber(self.BUF_VAL,
            self.tr('Fixed buf value (negative value to ignore)'),
            -1, 9999, -1))
        self.addParameter(ParameterSelection(self.RTYPE,
            self.tr('Output raster type'), self.TYPE, 5))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Distance')))

    def processAlgorithm(self, progress):
        output = self.getOutputValue(self.OUTPUT)

        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(output)

        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(output))

        arguments.append('-distunits')
        arguments.append(self.DISTUNITS[self.getParameterValue(self.UNITS)])

        values = self.getParameterValue(self.VALUES)
        if len(values) > 0:
            arguments.append('-values')
            arguments.append(values)

        values = str(self.getParameterValue(self.MAX_DIST))
        if values < 0:
            arguments.append('-maxdist')
            arguments.append(values)

        values = str(self.getParameterValue(self.NODATA))
        if values < 0:
            arguments.append('-nodata')
            arguments.append(values)

        values = str(self.getParameterValue(self.BUF_VAL))
        if values < 0:
            arguments.append('-fixed-buf-val')
            arguments.append(values)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_proximity.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_proximity.py',
                        GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
