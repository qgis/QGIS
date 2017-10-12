# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipRasterByMask.py
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

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipRasterByMask(GdalAlgorithm):

    INPUT = 'INPUT'
    MASK = 'MASK'
    NODATA = 'NODATA'
    ALPHA_BAND = 'ALPHA_BAND'
    CROP_TO_CUTLINE = 'CROP_TO_CUTLINE'
    KEEP_RESOLUTION = 'KEEP_RESOLUTION'
    OPTIONS = 'OPTIONS'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.MASK,
                                                              self.tr('Mask layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('Assign a specified nodata value to output bands'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.0,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.ALPHA_BAND,
                                                        self.tr('Create and output alpha band'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.CROP_TO_CUTLINE,
                                                        self.tr('Crop the extent of the target dataset to the extent of the cutline'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.KEEP_RESOLUTION,
                                                        self.tr('Keep resolution of output raster'),
                                                        defaultValue=False))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation parameters'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        self.addParameter(options_param)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Clipped (mask)')))

    def name(self):
        return 'cliprasterbymasklayer'

    def displayName(self):
        return self.tr('Clip raster by mask layer')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-clip.png'))

    def group(self):
        return self.tr('Raster extraction')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)

        maskLayer, maskLayerName = self.getOgrCompatibleSource(self.MASK, parameters, context, feedback)

        nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        if self.parameterAsBool(parameters, self.KEEP_RESOLUTION, context):
            arguments.append('-tr')
            arguments.append(str(inLayer.rasterUnitsPerPixelX()))
            arguments.append(str(-inLayer.rasterUnitsPerPixelY()))
            arguments.append('-tap')

        arguments.append('-cutline')
        arguments.append(maskLayer)

        if self.parameterAsBool(parameters, self.CROP_TO_CUTLINE, context):
            arguments.append('-crop_to_cutline')

        if self.parameterAsBool(parameters, self.ALPHA_BAND, context):
            arguments.append('-dstalpha')

        if nodata:
            arguments.append('-dstnodata {}'.format(nodata))

        if options:
            arguments.append('-co')
            arguments.append(options)

        arguments.append(inLayer.source())
        arguments.append(out)

        return ['gdalwarp', GdalUtils.escapeAndJoin(arguments)]
