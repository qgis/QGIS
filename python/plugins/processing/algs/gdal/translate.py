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
                                        ParameterString,
                                        ParameterNumber,
                                        ParameterBoolean,
                                        ParameterSelection,
                                        ParameterExtent,
                                        ParameterCrs)
from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


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
    RTYPE = 'RTYPE'
    OPTIONS = 'OPTIONS'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'translate.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterNumber(self.OUTSIZE,
                                          self.tr('Set the size of the output file (In pixels or %)'),
                                          1, None, 100))
        self.addParameter(ParameterBoolean(self.OUTSIZE_PERC,
                                           self.tr('Output size is a percentage of input size'), True))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          '', optional=True))
        self.addParameter(ParameterSelection(self.EXPAND,
                                             self.tr('Expand'), ['none', 'gray', 'rgb', 'rgba'], default=0))
        self.addParameter(ParameterCrs(self.SRS,
                                       self.tr('Output projection for output file [leave blank to use input projection]'), None, optional=True))
        self.addParameter(ParameterExtent(self.PROJWIN,
                                          self.tr('Subset based on georeferenced coordinates'), optional=True))
        self.addParameter(ParameterBoolean(self.SDS,
                                           self.tr('Copy all subdatasets of this file to individual output files'),
                                           False))

        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Converted')))

    def name(self):
        return 'translate'

    def displayName(self):
        return self.tr('Translate (convert format)')

    def group(self):
        return self.tr('Raster conversion')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.getParameterValue(self.INPUT)
        out = self.getOutputValue(translate.OUTPUT)
        outsize = str(self.getParameterValue(self.OUTSIZE))
        outsizePerc = str(self.getParameterValue(self.OUTSIZE_PERC))
        noData = self.getParameterValue(self.NO_DATA)
        expand = parameters[self.EXPAND].options[self.getParameterValue(self.EXPAND)][1]
        projwin = str(self.getParameterValue(self.PROJWIN))
        if not projwin:
            projwin = QgsProcessingUtils.combineLayerExtents([inLayer])
        crsId = self.getParameterValue(self.SRS)
        sds = self.getParameterValue(self.SDS)
        opts = self.getParameterValue(self.OPTIONS)

        if noData is not None:
            noData = str(noData)

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
        if noData and len(noData) > 0:
            arguments.append('-a_nodata')
            arguments.append(noData)
        if expand != 'none':
            arguments.append('-expand')
            arguments.append(expand)
        regionCoords = projwin.split(',')
        try:
            projwin = []
            projwin.append('-projwin')
            projwin.append(regionCoords[0])
            projwin.append(regionCoords[3])
            projwin.append(regionCoords[1])
            projwin.append(regionCoords[2])
        except IndexError:
            projwin = []
        if projwin:
            arguments.extend(projwin)
        if crsId:
            arguments.append('-a_srs')
            arguments.append(str(crsId))
        if sds:
            arguments.append('-sds')

        if opts:
            arguments.append('-co')
            arguments.append(opts)

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdal_translate', GdalUtils.escapeAndJoin(arguments)]
