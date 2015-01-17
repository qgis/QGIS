# -*- coding: utf-8 -*-

"""
***************************************************************************
    translate.py
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

from qgis.core import *
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalUtils import GdalUtils


class translate(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    OUTSIZE = 'OUTSIZE'
    OUTSIZE_PERC = 'OUTSIZE_PERC'
    NO_DATA = 'NO_DATA'
    EXPAND = 'EXPAND'
    PROJWIN = 'PROJWIN'
    SRS = 'SRS'
    SDS = 'SDS'
    EXTRA = 'EXTRA'
    RTYPE = 'RTYPE'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64',
            'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def commandLineName(self):
        return "gdalogr:translate"

    def defineCharacteristics(self):
        self.name = 'Translate (convert format)'
        self.group = '[GDAL] Conversion'
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'),
                          False))
        self.addParameter(ParameterNumber(self.OUTSIZE,
            self.tr('Set the size of the output file (In pixels or %)'),
            1, None, 100))
        self.addParameter(ParameterBoolean(self.OUTSIZE_PERC,
            self.tr('Output size is a percentage of input size'), True))
        self.addParameter(ParameterString(self.NO_DATA,
            self.tr("Nodata value, leave as 'none' to take the nodata value from input"),
            'none'))
        self.addParameter(ParameterSelection(self.EXPAND,
            self.tr('Expand'), ['none', 'gray', 'rgb', 'rgba']))
        self.addParameter(ParameterCrs(self.SRS,
            self.tr('Output projection for output file [leave blank to use input projection]'), None))
        self.addParameter(ParameterExtent(self.PROJWIN,
            self.tr('Subset based on georeferenced coordinates')))
        self.addParameter(ParameterBoolean(self.SDS,
            self.tr('Copy all subdatasets of this file to individual output files'),
            False))
        self.addParameter(ParameterString(self.EXTRA,
            self.tr('Additional creation parameters'), '', optional=True))
        self.addParameter(ParameterSelection(self.RTYPE,
            self.tr('Output raster type'), self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        out = self.getOutputValue(translate.OUTPUT)
        outsize = str(self.getParameterValue(self.OUTSIZE))
        outsizePerc = str(self.getParameterValue(self.OUTSIZE_PERC))
        noData = str(self.getParameterValue(self.NO_DATA))
        expand = str(self.getParameterFromName(
                self.EXPAND).options[self.getParameterValue(self.EXPAND)])
        projwin = str(self.getParameterValue(self.PROJWIN))
        crsId = self.getParameterValue(self.SRS)
        sds = self.getParameterValue(self.SDS)
        extra = str(self.getParameterValue(self.EXTRA))

        arguments = []
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if outsizePerc == 'True':
            arguments.append('-outsize')
            arguments.append(outsize + '%')
            arguments.append(outsize + '%')
        else:
            arguments.append('-outsize')
            arguments.append(outsize)
            arguments.append(outsize)
        arguments.append('-a_nodata')
        arguments.append(noData)
        if expand != 'none':
            arguments.append('-expand')
            arguments.append(expand)
        regionCoords = projwin.split(',')
        arguments.append('-projwin')
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[3])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[2])
        if crsId:
            arguments.append('-a_srs')
            arguments.append(str(crsId))
        if sds:
            arguments.append('-sds')
        if len(extra) > 0:
            arguments.append(extra)
        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        GdalUtils.runGdal(['gdal_translate',
                          GdalUtils.escapeAndJoin(arguments)], progress)
