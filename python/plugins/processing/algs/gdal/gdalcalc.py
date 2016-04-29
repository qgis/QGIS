# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdalcalc.py
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

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputRaster

from processing.tools.system import isWindows

from processing.algs.gdal.GdalUtils import GdalUtils


class gdalcalc(GdalAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'
    INPUT_C = 'INPUT_C'
    INPUT_D = 'INPUT_D'
    INPUT_E = 'INPUT_E'
    INPUT_F = 'INPUT_F'
    BAND_A = 'BAND_A'
    BAND_B = 'BAND_B'
    BAND_C = 'BAND_C'
    BAND_D = 'BAND_D'
    BAND_E = 'BAND_E'
    BAND_F = 'BAND_F'
    FORMULA = 'FORMULA'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    EXTRA = 'EXTRA'
    RTYPE = 'RTYPE'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']
    #DEBUG = 'DEBUG'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Raster calculator')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Miscellaneous')
        self.addParameter(ParameterRaster(
            self.INPUT_A, self.tr('Input layer A'), False))
        self.addParameter(ParameterString(self.BAND_A,
                                          self.tr('Number of raster band for raster A'), '1', optional=True))
        self.addParameter(ParameterRaster(
            self.INPUT_B, self.tr('Input layer B'), True))
        self.addParameter(ParameterString(self.BAND_B,
                                          self.tr('Number of raster band for raster B'), '1', optional=True))
        self.addParameter(ParameterRaster(
            self.INPUT_C, self.tr('Input layer C'), True))
        self.addParameter(ParameterString(self.BAND_C,
                                          self.tr('Number of raster band for raster C'), '1', optional=True))
        self.addParameter(ParameterRaster(
            self.INPUT_D, self.tr('Input layer D'), True))
        self.addParameter(ParameterString(self.BAND_D,
                                          self.tr('Number of raster band for raster D'), '1', optional=True))
        self.addParameter(ParameterRaster(
            self.INPUT_E, self.tr('Input layer E'), True))
        self.addParameter(ParameterString(self.BAND_E,
                                          self.tr('Number of raster band for raster E'), '1', optional=True))
        self.addParameter(ParameterRaster(
            self.INPUT_F, self.tr('Input layer F'), True))
        self.addParameter(ParameterString(self.BAND_F,
                                          self.tr('Number of raster band for raster F'), '1', optional=True))
        self.addParameter(ParameterString(self.FORMULA,
                                          self.tr('Calculation in gdalnumeric syntax using +-/* or any numpy array functions (i.e. logical_and())'), 'A*2', optional=False))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr('Set output nodata value'), '', optional=True))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'), self.TYPE, 5))
        #self.addParameter(ParameterBoolean(
        #    self.DEBUG, self.tr('Print debugging information'), False))
        self.addParameter(ParameterString(self.EXTRA,
                                          self.tr('Additional creation parameters'), '', optional=True))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Calculated')))

    def getConsoleCommands(self):
        out = self.getOutputValue(self.OUTPUT)
        extra = self.getParameterValue(self.EXTRA)
        if extra is not None:
            extra = unicode(extra)
        #debug = self.getParameterValue(self.DEBUG)
        formula = self.getParameterValue(self.FORMULA)
        noData = self.getParameterValue(self.NO_DATA)
        if noData is not None:
            noData = unicode(noData)

        arguments = []
        arguments.append('--calc')
        arguments.append('"' + formula + '"')
        arguments.append('--format')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append('--type')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if noData and len(noData) > 0:
            arguments.append('--NoDataValue')
            arguments.append(noData)
        if extra and len(extra) > 0:
            arguments.append(extra)
        #if debug:
        #    arguments.append('--debug')
        arguments.append('-A')
        arguments.append(self.getParameterValue(self.INPUT_A))
        if self.getParameterValue(self.BAND_A):
            arguments.append('--A_band ' + self.getParameterValue(self.BAND_A))
        if self.getParameterValue(self.INPUT_B):
            arguments.append('-B')
            arguments.append(self.getParameterValue(self.INPUT_B))
            if self.getParameterValue(self.BAND_B):
                arguments.append('--B_band ' + self.getParameterValue(self.BAND_B))
        if self.getParameterValue(self.INPUT_C):
            arguments.append('-C')
            arguments.append(self.getParameterValue(self.INPUT_C))
            if self.getParameterValue(self.BAND_C):
                arguments.append('--C_band ' + self.getParameterValue(self.BAND_C))
        if self.getParameterValue(self.INPUT_D):
            arguments.append('-D')
            arguments.append(self.getParameterValue(self.INPUT_D))
            if self.getParameterValue(self.BAND_D):
                arguments.append('--D_band ' + self.getParameterValue(self.BAND_D))
        if self.getParameterValue(self.INPUT_E):
            arguments.append('-E')
            arguments.append(self.getParameterValue(self.INPUT_E))
            if self.getParameterValue(self.BAND_E):
                arguments.append('--E_band ' + self.getParameterValue(self.BAND_E))
        if self.getParameterValue(self.INPUT_F):
            arguments.append('-F')
            arguments.append(self.getParameterValue(self.INPUT_F))
            if self.getParameterValue(self.BAND_F):
                arguments.append('--F_band ' + self.getParameterValue(self.BAND_F))
        arguments.append('--outfile')
        arguments.append(out)

        if isWindows():
            return ['gdal_calc', GdalUtils.escapeAndJoin(arguments)]
        else:
            return ['gdal_calc.py', GdalUtils.escapeAndJoin(arguments)]
