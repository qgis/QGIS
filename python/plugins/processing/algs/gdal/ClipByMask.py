# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipByMask.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from osgeo import gdal

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString

from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalUtils import GdalUtils


class ClipByMask(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    MASK = 'MASK'
    ALPHA_BAND = 'ALPHA_BAND'
    KEEP_RESOLUTION = 'KEEP_RESOLUTION'
    EXTRA = 'EXTRA'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip raster by mask layer')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Extraction')
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterVector(self.MASK, self.tr('Mask layer'),
                          [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterString(self.NO_DATA,
            self.tr("Nodata value, leave blank to take the nodata value from input"),
            '-9999'))
        self.addParameter(ParameterBoolean(self.ALPHA_BAND,
            self.tr('Create and output alpha band'), False))
        self.addParameter(ParameterBoolean(self.KEEP_RESOLUTION,
            self.tr('Keep resolution of output raster'), False))
        self.addParameter(ParameterString(self.EXTRA,
            self.tr('Additional creation parameters'), '', optional=True))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Clipped (mask)')))

    def getConsoleCommands(self):
        out = self.getOutputValue(self.OUTPUT)
        mask = self.getParameterValue(self.MASK)
        noData = str(self.getParameterValue(self.NO_DATA))
        addAlphaBand = self.getParameterValue(self.ALPHA_BAND)
        keepResolution = self.getParameterValue(self.KEEP_RESOLUTION)
        extra = str(self.getParameterValue(self.EXTRA))

        arguments = []
        arguments.append('-q')
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        if len(noData) > 0:
            arguments.append('-dstnodata')
            arguments.append(noData)

        if keepResolution:
            r = gdal.Open(self.getParameterValue(self.INPUT))
            geoTransform = r.GetGeoTransform()
            r = None
            arguments.append('-tr')
            arguments.append(str(geoTransform[1]))
            arguments.append(str(geoTransform[5]))
            arguments.append('-tap')

        arguments.append('-cutline')
        arguments.append(mask)
        arguments.append('-crop_to_cutline')

        if addAlphaBand:
            arguments.append('-dstalpha')

        if len(extra) > 0:
            arguments.append(extra)

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
