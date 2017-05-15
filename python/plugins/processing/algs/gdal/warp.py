# -*- coding: utf-8 -*-

"""
***************************************************************************
    self.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import (ParameterRaster,
                                        ParameterExtent,
                                        ParameterSelection,
                                        ParameterCrs,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterBoolean)
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class warp(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SOURCE_SRS = 'SOURCE_SRS'
    DEST_SRS = 'DEST_SRS'
    METHOD = 'METHOD'
    TR = 'TR'
    NO_DATA = 'NO_DATA'
    USE_RASTER_EXTENT = 'USE_RASTER_EXTENT'
    RASTER_EXTENT = 'RASTER_EXTENT'
    EXTENT_CRS = 'EXTENT_CRS'
    RTYPE = 'RTYPE'
    OPTIONS = 'OPTIONS'
    MULTITHREADING = 'MULTITHREADING'

    METHOD_OPTIONS = ['near', 'bilinear', 'cubic', 'cubicspline', 'lanczos']
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'warp.png'))

    def tags(self):
        return self.tr('transform,reproject,crs,srs').split(',')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterCrs(self.SOURCE_SRS,
                                       self.tr('Source SRS'),
                                       '',
                                       optional=True))
        self.addParameter(ParameterCrs(self.DEST_SRS,
                                       self.tr('Destination SRS'),
                                       'EPSG:4326'))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          '', optional=True))
        self.addParameter(ParameterNumber(self.TR,
                                          self.tr('Output file resolution in target georeferenced units (leave 0 for no change)'),
                                          0.0, None, 0.0))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Resampling method'),
                                             self.METHOD_OPTIONS))
        self.addParameter(ParameterBoolean(self.USE_RASTER_EXTENT,
                                           self.tr('Set georeferenced extents of output file'),
                                           False
                                           ))
        self.addParameter(ParameterExtent(self.RASTER_EXTENT,
                                          self.tr('Raster extent'),
                                          optional=True))
        self.addParameter(ParameterCrs(self.EXTENT_CRS,
                                       self.tr('CRS of the raster extent, leave blank for using Destination SRS'),
                                       optional=True))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterBoolean(self.MULTITHREADING,
                                           self.tr('Use multithreaded warping implementation'),
                                           False
                                           ))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Reprojected')))

    def name(self):
        return 'warpreproject'

    def displayName(self):
        return self.tr('Warp (reproject)')

    def group(self):
        return self.tr('Raster projections')

    def getConsoleCommands(self):
        srccrs = self.getParameterValue(self.SOURCE_SRS)
        dstcrs = self.getParameterValue(self.DEST_SRS)
        useRasterExtent = self.getParameterValue(self.USE_RASTER_EXTENT)
        rasterExtent = self.getParameterValue(self.RASTER_EXTENT)
        extentCrs = self.getParameterValue(self.EXTENT_CRS)
        opts = self.getParameterValue(self.OPTIONS)
        noData = self.getParameterValue(self.NO_DATA)
        multithreading = self.getParameterValue(self.MULTITHREADING)

        if noData is not None:
            noData = str(noData)

        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if srccrs:
            arguments.append('-s_srs')
            arguments.append(srccrs)
        if dstcrs:
            arguments.append('-t_srs')
            arguments.append(dstcrs)
        if noData:
            arguments.append('-dstnodata')
            arguments.append(noData)

        arguments.append('-r')
        arguments.append(
            self.METHOD_OPTIONS[self.getParameterValue(self.METHOD)])

        arguments.append('-of')
        out = self.getOutputValue(self.OUTPUT)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))

        if self.getParameterValue(self.TR) != 0:
            arguments.append('-tr')
            arguments.append(str(self.getParameterValue(self.TR)))
            arguments.append(str(self.getParameterValue(self.TR)))

        if useRasterExtent:
            regionCoords = rasterExtent.split(',')
            if len(regionCoords) >= 4:
                arguments.append('-te')
                arguments.append(regionCoords[0])
                arguments.append(regionCoords[2])
                arguments.append(regionCoords[1])
                arguments.append(regionCoords[3])

                if extentCrs:
                    arguments.append('-te_srs')
                    arguments.append(extentCrs)

        if opts:
            arguments.append('-co')
            arguments.append(opts)

        if multithreading:
            arguments.append('-multi')

        if GdalUtils.version() in [2010000, 2010100]:
            arguments.append("--config GDALWARP_IGNORE_BAD_CUTLINE YES")

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
