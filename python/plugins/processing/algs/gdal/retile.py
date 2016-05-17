# -*- coding: utf-8 -*-

"""
***************************************************************************
    retile.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputDirectory
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils
import re


class retile(GdalAlgorithm):

    INPUT = 'INPUT'
    RTYPE = 'RTYPE'
    ONLYPYRAMIDS = 'ONLYPYRAMIDS'
    PYRAMIDLEVELS = 'PYRAMIDLEVELS'
    PIXELSIZE = 'PIXELSIZE'
    ALGORITHM = 'ALGORITHM'
    USEDIRFOREACHROW = 'USEDIRFOREACHROW'
    S_SRS = 'S_SRS'
    TARGETDIR = 'TARGETDIR'
    CSVFILE = 'CSVFILE'
    CSVDELIM = 'CSVDELIM'
    TILEINDEX = 'TILEINDEX'
    TILEINDEXFIELD = 'TILEINDEXFIELD'
    FORMAT = 'FORMAT'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']
    ALGO = ['near', 'bilinear', 'cubic', 'cubicspline', 'lanczos']

    def commandLineName(self):
        return "gdalogr:retile"

    def commandName(self):
        return "gdal_retile"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Retile')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Miscellaneous')

        # Required parameters
        self.addParameter(ParameterMultipleInput(self.INPUT,
                                                 self.tr('Input layers'),
                                                 ParameterMultipleInput.TYPE_RASTER))
        # Advanced parameters
        params = []
        params.append(ParameterString(self.PIXELSIZE,
                                      self.tr('Pixel size to be used for the output file (XSIZE YSIZE like 512 512)'),
                                      None, False, True))
        params.append(ParameterSelection(self.ALGORITHM,
                                         self.tr('Resampling algorithm'), self.ALGO, 0, False, True))
        params.append(ParameterCrs(self.S_SRS,
                                   self.tr('Override source CRS'), None, True))
        params.append(ParameterNumber(self.PYRAMIDLEVELS,
                                      self.tr('Number of pyramids levels to build'),
                                      None, None, None, True))
        params.append(ParameterBoolean(self.ONLYPYRAMIDS,
                                       self.tr('Build only the pyramids'),
                                       False, True))
        params.append(ParameterSelection(self.RTYPE,
                                         self.tr('Output raster type'),
                                         self.TYPE, 5, False, True))
        params.append(ParameterSelection(self.FORMAT,
                                         self.tr('Output raster format'),
                                         GdalUtils.getSupportedRasters().keys(), 0, False, True))
        params.append(ParameterBoolean(self.USEDIRFOREACHROW,
                                       self.tr('Use a directory for each row'),
                                       False, True))
        params.append(ParameterString(self.CSVFILE,
                                      self.tr('Name of the csv file containing the tile(s) georeferencing information'),
                                      None, False, True))
        params.append(ParameterString(self.CSVDELIM,
                                      self.tr('Column delimiter used in the CSV file'),
                                      None, False, True))
        params.append(ParameterString(self.TILEINDEX,
                                      self.tr('name of shape file containing the result tile(s) index'),
                                      None, False, True))
        params.append(ParameterString(self.TILEINDEXFIELD,
                                      self.tr('name of the attribute containing the tile name in the result shape file'),
                                      None, False, True))

        for param in params:
            param.isAdvanced = True
            self.addParameter(param)

        self.addOutput(OutputDirectory(self.TARGETDIR,
                                       self.tr('The directory where the tile result is created')))

    def getConsoleCommands(self):

        arguments = []

        if self.getParameterValue(self.RTYPE):
            arguments.append('-ot')
            arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])

        arguments.append('-of')
        arguments.append(GdalUtils.getSupportedRasters().keys()[self.getParameterValue(self.FORMAT)])

        if self.getParameterValue(self.PIXELSIZE):
            pixelSize = self.getParameterValue(self.PIXELSIZE)
            if re.match(r'\d+ \d+', pixelSize):
                xsize, ysize = pixelSize.split(' ')
                arguments.append('-ps')
                arguments.append(xsize)
                arguments.append(ysize)

        if self.getParameterValue(self.ONLYPYRAMIDS):
            arguments.append('-pyramidOnly')

        if self.getParameterValue(self.USEDIRFOREACHROW):
            arguments.append('-useDirForEachRow')

        ssrs = unicode(self.getParameterValue(self.S_SRS))
        if len(ssrs) > 0:
            arguments.append('-s_srs')
            arguments.append(ssrs)

        if self.getParameterValue(self.PYRAMIDLEVELS):
            arguments.append('-levels')
            arguments.append(unicode(self.getParameterValue(self.PYRAMIDLEVELS)))

        arguments.append('-r')
        arguments.append(self.ALGO[self.getParameterValue(self.ALGORITHM)])

        # Handle CSV
        if self.getParameterValue(self.CSVFILE):
            arguments.append('-csv')
            arguments.append(self.getParameterValue(self.CSVFILE))

        if self.getParameterValue(self.CSVFILE) and self.getParameterValue(self.CSVDELIM):
            arguments.append('-csvDelim')
            arguments.append(self.getParameterValue(self.CSVDELIM))

        # Handle Shp
        if self.getParameterValue(self.TILEINDEX):
            arguments.append('-tileIndex')
            arguments.append(self.getParameterValue(self.TILEINDEX))

        if self.getParameterValue(self.TILEINDEX) and self.getParameterValue(self.TILEINDEXFIELD):
            arguments.append('-tileIndexField')
            arguments.append(self.getParameterValue(self.TILEINDEXFIELD))

        arguments.append('-targetDir')
        arguments.append(self.getOutputValue(self.TARGETDIR))

        arguments.extend(self.getParameterValue(self.INPUT).split(';'))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_retile.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_retile.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
