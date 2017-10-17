# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipByExtent.py
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

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.core.parameters import (ParameterRaster,
                                        ParameterExtent,
                                        ParameterString,
                                        ParameterSelection)
from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipByExtent(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    PROJWIN = 'PROJWIN'
    OPTIONS = 'OPTIONS'
    RTYPE = 'RTYPE'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          '',
                                          optional=True))
        self.addParameter(ParameterExtent(self.PROJWIN, self.tr('Clipping extent')))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Clipped (extent)')))

    def name(self):
        return 'cliprasterbyextent'

    def displayName(self):
        return self.tr('Clip raster by extent')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-clip.png'))

    def group(self):
        return self.tr('Raster extraction')

    def getConsoleCommands(self, parameters, context, feedback):
        out = self.getOutputValue(self.OUTPUT)
        noData = self.getParameterValue(self.NO_DATA)
        opts = self.getParameterValue(self.OPTIONS)
        projwin = self.getParameterValue(self.PROJWIN)
        layer = self.getParameterValue(self.INPUT)
        if not projwin:
            projwin = QgsProcessingUtils.combineLayerExtents([layer])

        if noData is not None:
            noData = str(noData)

        arguments = []
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if noData and len(noData) > 0:
            arguments.append('-a_nodata')
            arguments.append(noData)

        regionCoords = projwin.split(',')
        arguments.append('-projwin')
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[3])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[2])

        if opts:
            arguments.append('-co')
            arguments.append(opts)

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdal_translate', GdalUtils.escapeAndJoin(arguments)]

    def commandName(self):
        return "gdal_translate"
