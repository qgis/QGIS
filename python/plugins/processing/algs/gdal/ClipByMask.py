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

import os

from qgis.PyQt.QtGui import QIcon

from osgeo import gdal

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber

from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools import dataobjects
from processing.tools.vector import ogrConnectionString

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipByMask(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    MASK = 'MASK'
    ALPHA_BAND = 'ALPHA_BAND'
    CROP_TO_CUTLINE = 'CROP_TO_CUTLINE'
    KEEP_RESOLUTION = 'KEEP_RESOLUTION'
    EXTRA = 'EXTRA'
    RTYPE = 'RTYPE'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']
    TILED = 'TILED'
    COMPRESS = 'COMPRESS'
    JPEGCOMPRESSION = 'JPEGCOMPRESSION'
    PREDICTOR = 'PREDICTOR'
    ZLEVEL = 'ZLEVEL'
    BIGTIFF = 'BIGTIFF'
    BIGTIFFTYPE = ['', 'YES', 'NO', 'IF_NEEDED', 'IF_SAFER']
    COMPRESSTYPE = ['NONE', 'JPEG', 'LZW', 'PACKBITS', 'DEFLATE']
    TFW = 'TFW'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-clip.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip raster by mask layer')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Extraction')
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterVector(self.MASK, self.tr('Mask layer'),
                                          [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          '', optional=True))
        self.addParameter(ParameterBoolean(self.ALPHA_BAND,
                                           self.tr('Create and output alpha band'), False))
        self.addParameter(ParameterBoolean(self.CROP_TO_CUTLINE,
                                           self.tr('Crop the extent of the target dataset to the extent of the cutline'), False))
        self.addParameter(ParameterBoolean(self.KEEP_RESOLUTION,
                                           self.tr('Keep resolution of output raster'), False))

        params = []
        params.append(ParameterSelection(self.RTYPE,
                                         self.tr('Output raster type'), self.TYPE, 5))
        params.append(ParameterSelection(self.COMPRESS,
                                         self.tr('GeoTIFF options. Compression type:'), self.COMPRESSTYPE, 4))
        params.append(ParameterNumber(self.JPEGCOMPRESSION,
                                      self.tr('Set the JPEG compression level'),
                                      1, 100, 75))
        params.append(ParameterNumber(self.ZLEVEL,
                                      self.tr('Set the DEFLATE compression level'),
                                      1, 9, 6))
        params.append(ParameterNumber(self.PREDICTOR,
                                      self.tr('Set the predictor for LZW or DEFLATE compression'),
                                      1, 3, 1))
        params.append(ParameterBoolean(self.TILED,
                                       self.tr('Create tiled output (only used for the GTiff format)'), False))
        params.append(ParameterSelection(self.BIGTIFF,
                                         self.tr('Control whether the created file is a BigTIFF or a classic TIFF'), self.BIGTIFFTYPE, 0))
        params.append(ParameterBoolean(self.TFW,
                                       self.tr('Force the generation of an associated ESRI world file (.tfw))'), False))
        params.append(ParameterString(self.EXTRA,
                                      self.tr('Additional creation parameters'), '', optional=True))

        for param in params:
            param.isAdvanced = True
            self.addParameter(param)

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Clipped (mask)')))

    def getConsoleCommands(self):
        out = self.getOutputValue(self.OUTPUT)
        mask = self.getParameterValue(self.MASK)
        maskLayer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.MASK))
        ogrMask = ogrConnectionString(mask)[1:-1]
        noData = self.getParameterValue(self.NO_DATA)
        if noData is not None:
            noData = unicode(noData)
        addAlphaBand = self.getParameterValue(self.ALPHA_BAND)
        cropToCutline = self.getParameterValue(self.CROP_TO_CUTLINE)
        keepResolution = self.getParameterValue(self.KEEP_RESOLUTION)
        extra = self.getParameterValue(self.EXTRA)
        if extra is not None:
            extra = unicode(extra)
        jpegcompression = unicode(self.getParameterValue(self.JPEGCOMPRESSION))
        predictor = unicode(self.getParameterValue(self.PREDICTOR))
        zlevel = unicode(self.getParameterValue(self.ZLEVEL))
        tiled = unicode(self.getParameterValue(self.TILED))
        compress = self.COMPRESSTYPE[self.getParameterValue(self.COMPRESS)]
        bigtiff = self.BIGTIFFTYPE[self.getParameterValue(self.BIGTIFF)]
        tfw = unicode(self.getParameterValue(self.TFW))

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
            arguments.append(unicode(geoTransform[1]))
            arguments.append(unicode(geoTransform[5]))
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

        if extra and len(extra) > 0:
            arguments.append(extra)
        if GdalUtils.getFormatShortNameFromFilename(out) == "GTiff":
            arguments.append("-co COMPRESS=" + compress)
            if compress == 'JPEG':
                arguments.append("-co JPEG_QUALITY=" + jpegcompression)
            elif (compress == 'LZW') or (compress == 'DEFLATE'):
                arguments.append("-co PREDICTOR=" + predictor)
            if compress == 'DEFLATE':
                arguments.append("-co ZLEVEL=" + zlevel)
            if tiled == "True":
                arguments.append("-co TILED=YES")
            if tfw == "True":
                arguments.append("-co TFW=YES")
            if len(bigtiff) > 0:
                arguments.append("-co BIGTIFF=" + bigtiff)

            arguments.append("-wo OPTIMIZE_SIZE=TRUE")

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
