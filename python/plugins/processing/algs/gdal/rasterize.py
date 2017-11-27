# -*- coding: utf-8 -*-

"""
***************************************************************************
    rasterize.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
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

from processing.core.parameters import (ParameterVector,
                                        ParameterExtent,
                                        ParameterTableField,
                                        ParameterSelection,
                                        ParameterNumber,
                                        ParameterString)
from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rasterize(GdalAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    DIMENSIONS = 'DIMENSIONS'
    WIDTH = 'WIDTH'
    HEIGHT = 'HEIGHT'
    NO_DATA = 'NO_DATA'
    RTYPE = 'RTYPE'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    RAST_EXT = 'RAST_EXT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'rasterize.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Attribute field'), self.INPUT))
        self.addParameter(ParameterSelection(self.DIMENSIONS,
                                             self.tr('Set output raster size (ignored if above option is checked)'),
                                             ['Output size in pixels', 'Output resolution in map units per pixel'], 1))
        self.addParameter(ParameterNumber(self.WIDTH,
                                          self.tr('Horizontal'), 0.0, 99999999.999999, 100.0))
        self.addParameter(ParameterNumber(self.HEIGHT,
                                          self.tr('Vertical'), 0.0, 99999999.999999, 100.0))
        self.addParameter(ParameterExtent(self.RAST_EXT, self.tr('Raster extent')))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value"),
                                          '', optional=True))

        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT,
                                    self.tr('Rasterized')))

    def name(self):
        return 'rasterize'

    def displayName(self):
        return self.tr('Rasterize (vector to raster)')

    def group(self):
        return self.tr('Vector conversion')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.getParameterValue(self.INPUT)
        noData = self.getParameterValue(self.NO_DATA)
        rastext = str(self.getParameterValue(self.RAST_EXT))
        if not rastext:
            rastext = QgsProcessingUtils.combineLayerExtents([inLayer])
        opts = self.getParameterValue(self.OPTIONS)
        out = self.getOutputValue(self.OUTPUT)

        ogrLayer = GdalUtils.ogrConnectionString(inLayer, context)[1:-1]

        if noData is not None:
            noData = str(noData)

        arguments = []
        arguments.append('-a')
        arguments.append(str(self.getParameterValue(self.FIELD)))

        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        dimType = self.getParameterValue(self.DIMENSIONS)
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))

        regionCoords = rastext.split(',')
        try:
            rastext = []
            rastext.append('-te')
            rastext.append(regionCoords[0])
            rastext.append(regionCoords[2])
            rastext.append(regionCoords[1])
            rastext.append(regionCoords[3])
        except IndexError:
            rastext = []
        if rastext:
            arguments.extend(rastext)

        if dimType == 0:
            # size in pixels
            arguments.append('-ts')
            arguments.append(str(self.getParameterValue(self.WIDTH)))
            arguments.append(str(self.getParameterValue(self.HEIGHT)))
        else:
            # resolution in map units per pixel
            arguments.append('-tr')
            arguments.append(str(self.getParameterValue(self.WIDTH)))
            arguments.append(str(self.getParameterValue(self.HEIGHT)))

        if noData and len(noData) > 0:
            arguments.append('-a_nodata')
            arguments.append(noData)

        if opts:
            arguments.append('-co')
            arguments.append(opts)

        arguments.append('-l')

        print(GdalUtils.ogrLayerName(inLayer))
        arguments.append(GdalUtils.ogrLayerName(inLayer))
        arguments.append(ogrLayer)

        arguments.append(out)
        return ['gdal_rasterize', GdalUtils.escapeAndJoin(arguments)]

    def commandName(self):
        return "gdal_rasterize"
