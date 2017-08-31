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

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingUtils)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

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
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterNumber(self.OUTSIZE,
                                                       self.tr('Set the size of the output file (In pixels or %)'),
                                                       minValue=1, defaultValue=100))
        self.addParameter(QgsProcessingParameterBoolean(self.OUTSIZE_PERC,
                                                        self.tr('Output size is a percentage of input size'), defaultValue=True))
        self.addParameter(QgsProcessingParameterString(self.NO_DATA,
                                                       self.tr("Nodata value, leave blank to take the nodata value from input"),
                                                       defaultValue='', optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.EXPAND,
                                                     self.tr('Expand'), options=['none', 'gray', 'rgb', 'rgba'], defaultValue=0))
        self.addParameter(QgsProcessingParameterCrs(self.SRS,
                                                    self.tr('Output projection for output file [leave blank to use input projection]'), defaultValue=None, optional=True))
        self.addParameter(QgsProcessingParameterExtent(self.PROJWIN,
                                                       self.tr('Subset based on georeferenced coordinates'), optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.SDS,
                                                        self.tr('Copy all subdatasets of this file to individual output files'),
                                                        defaultValue=False))

        create_options_param = QgsProcessingParameterString(self.OPTIONS,
                                                            self.tr('Additional creation options'),
                                                            optional=True)
        create_options_param.setMetadata({'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'})
        self.addParameter(create_options_param)
        self.addParameter(QgsProcessingParameterEnum(self.RTYPE,
                                                     self.tr('Output raster type'),
                                                     options=self.TYPE, defaultValue=5))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Converted')))

    def name(self):
        return 'translate'

    def displayName(self):
        return self.tr('Translate (convert format)')

    def group(self):
        return self.tr('Raster conversion')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        outsize = str(self.parameterAsInt(parameters, self.OUTSIZE, context))
        outsizePerc = self.parameterAsBool(parameters, self.OUTSIZE_PERC, context)
        noData = self.parameterAsString(parameters, self.NO_DATA, context)
        expand = self.parameterDefinition(self.EXPAND).options()[self.parameterAsEnum(parameters, self.EXPAND, context)]

        proj_extent = self.parameterAsExtent(parameters, self.PROJWIN, context)
        if proj_extent.isNull():
            proj_extent = QgsProcessingUtils.combineLayerExtents([inLayer])
        crsId = self.parameterAsCrs(parameters, self.SRS, context).authid()
        sds = self.parameterAsBool(parameters, self.SDS, context)
        opts = self.parameterAsString(parameters, self.OPTIONS, context)

        if noData is not None:
            noData = str(noData)

        arguments = []
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))
        arguments.append('-ot')
        arguments.append(self.TYPE[self.parameterAsEnum(parameters, self.RTYPE, context)])
        if outsizePerc:
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
        try:
            projwin = []
            projwin.append('-projwin')
            projwin.append(proj_extent.xMinimum())
            projwin.append(proj_extent.yMaximum())
            projwin.append(proj_extent.xMaximum())
            projwin.append(proj_extent.yMinimum())
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

        arguments.append(inLayer.source())
        arguments.append(out)

        return ['gdal_translate', GdalUtils.escapeAndJoin(arguments)]
