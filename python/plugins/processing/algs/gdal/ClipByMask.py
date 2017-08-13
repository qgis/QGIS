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
from builtins import str

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsProcessingUtils
from osgeo import gdal

from processing.core.parameters import (ParameterRaster,
                                        ParameterVector,
                                        ParameterBoolean,
                                        ParameterString,
                                        ParameterSelection)

from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipByMask(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    MASK = 'MASK'
    ALPHA_BAND = 'ALPHA_BAND'
    CROP_TO_CUTLINE = 'CROP_TO_CUTLINE'
    KEEP_RESOLUTION = 'KEEP_RESOLUTION'
    OPTIONS = 'OPTIONS'
    RTYPE = 'RTYPE'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterVector(self.MASK, self.tr('Mask layer'),
                                          [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          '', optional=True))
        self.addParameter(ParameterBoolean(self.ALPHA_BAND,
                                           self.tr('Create and output alpha band'),
                                           False))
        self.addParameter(ParameterBoolean(self.CROP_TO_CUTLINE,
                                           self.tr('Crop the extent of the target dataset to the extent of the cutline'),
                                           True))
        self.addParameter(ParameterBoolean(self.KEEP_RESOLUTION,
                                           self.tr('Keep resolution of output raster'),
                                           False))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Clipped (mask)')))

    def name(self):
        return 'cliprasterbymasklayer'

    def displayName(self):
        return self.tr('Clip raster by mask layer')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-clip.png'))

    def group(self):
        return self.tr('Raster extraction')

    def getConsoleCommands(self, parameters, context, feedback):
        out = self.getOutputValue(self.OUTPUT)
        mask = self.getParameterValue(self.MASK)
        context = dataobjects.createContext()
        maskLayer = QgsProcessingUtils.mapLayerFromString(self.getParameterValue(self.MASK), context)
        ogrMask = GdalUtils.ogrConnectionString(mask, context)[1:-1]
        noData = self.getParameterValue(self.NO_DATA)
        opts = self.getParameterValue(self.OPTIONS)

        if noData is not None:
            noData = str(noData)

        addAlphaBand = self.getParameterValue(self.ALPHA_BAND)
        cropToCutline = self.getParameterValue(self.CROP_TO_CUTLINE)
        keepResolution = self.getParameterValue(self.KEEP_RESOLUTION)

        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        arguments.append('-q')
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        if noData and len(noData) > 0:
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
        arguments.append(ogrMask)
        if maskLayer and maskLayer.subsetString() != '':
            arguments.append('-cwhere')
            arguments.append(maskLayer.subsetString())

        if cropToCutline:
            arguments.append('-crop_to_cutline')

        if addAlphaBand:
            arguments.append('-dstalpha')

        if opts:
            arguments.append('-co')
            arguments.append(opts)

        if GdalUtils.version() in [2010000, 2010100]:
            arguments.append("--config GDALWARP_IGNORE_BAD_CUTLINE YES")

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
