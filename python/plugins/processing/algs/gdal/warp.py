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
from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingUtils)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
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

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer'), optional=False))
        self.addParameter(QgsProcessingParameterCrs(self.SOURCE_SRS,
                                                    self.tr('Source SRS'),
                                                    optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.DEST_SRS,
                                                    self.tr('Destination SRS'),
                                                    'EPSG:4326'))
        self.addParameter(QgsProcessingParameterString(self.NO_DATA,
                                                       self.tr("Nodata value, leave blank to take the nodata value from input"), optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.TR,
                                                       self.tr('Output file resolution in target georeferenced units (leave 0 for no change)'),
                                                       minValue=0.0, defaultValue=0.0))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Resampling method'),
                                                     self.METHOD_OPTIONS))
        self.addParameter(QgsProcessingParameterBoolean(self.USE_RASTER_EXTENT,
                                                        self.tr('Set georeferenced extents of output file'),
                                                        False
                                                        ))
        self.addParameter(QgsProcessingParameterExtent(self.RASTER_EXTENT,
                                                       self.tr('Raster extent'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.EXTENT_CRS,
                                                    self.tr('CRS of the raster extent, leave blank for using Destination SRS'),
                                                    optional=True))
        co = QgsProcessingParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True)
        co.setMetadata({'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'})
        self.addParameter(co)
        self.addParameter(QgsProcessingParameterBoolean(self.MULTITHREADING,
                                                        self.tr('Use multithreaded warping implementation'),
                                                        False
                                                        ))
        self.addParameter(QgsProcessingParameterEnum(self.RTYPE,
                                                     self.tr('Output raster type'),
                                                     self.TYPE, defaultValue=5))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Reprojected')))

    def name(self):
        return 'warpreproject'

    def displayName(self):
        return self.tr('Warp (reproject)')

    def group(self):
        return self.tr('Raster projections')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        srccrs = self.parameterAsCrs(parameters, self.SOURCE_SRS, context).authid()
        dstcrs = self.parameterAsCrs(parameters, self.DEST_SRS, context).authid()
        useRasterExtent = self.parameterAsBool(parameters, self.USE_RASTER_EXTENT, context)
        rasterExtent = self.parameterAsExtent(parameters, self.RASTER_EXTENT, context)
        if rasterExtent.isNull():
            rasterExtent = QgsProcessingUtils.combineLayerExtents([inLayer])
        extentCrs = self.parameterAsCrs(parameters, self.EXTENT_CRS, context).authid()
        opts = self.parameterAsEnum(parameters, self.OPTIONS, context)
        noData = self.parameterAsString(parameters, self.NO_DATA, context)
        multithreading = self.parameterAsBool(parameters, self.MULTITHREADING, context)

        if noData is not None:
            noData = str(noData)

        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.parameterAsEnum(parameters, self.RTYPE, context)])
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
            self.METHOD_OPTIONS[self.parameterAsEnum(parameters, self.METHOD, context)])

        arguments.append('-of')
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))

        if self.parameterAsDouble(parameters, self.TR, context) != 0:
            arguments.append('-tr')
            arguments.append(str(self.parameterAsDouble(parameters, self.TR, context)))
            arguments.append(str(self.parameterAsDouble(parameters, self.TR, context)))

        if useRasterExtent:
            arguments.append('-te')
            arguments.append(rasterExtent.xMinimum())
            arguments.append(rasterExtent.yMinimum())
            arguments.append(rasterExtent.xMaximum())
            arguments.append(rasterExtent.yMaximum())

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

        arguments.append(inLayer.source())
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
