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
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterExtent,
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
    TARGET_EXTENT = 'TARGET_EXTENT'
    TARGET_EXTENT_CRS = 'TARGET_EXTENT_CRS'
    MULTITHREADING = 'MULTITHREADING'
    OUTPUT = 'OUTPUT'

    TYPES = ['Use input layer data type', 'Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

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
                                                       defaultValue=None,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.ALPHA_BAND,
                                                        self.tr('Create an output alpha band'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.CROP_TO_CUTLINE,
                                                        self.tr('Match the extent of the clipped raster to the extent of the mask layer'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.KEEP_RESOLUTION,
                                                        self.tr('Keep resolution of output raster'),
                                                        defaultValue=False))

        target_extent_param = QgsProcessingParameterExtent(self.TARGET_EXTENT,
                                                           self.tr('Georeferenced extents of output file to be created'),
                                                           optional=True)
        target_extent_param.setFlags(target_extent_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(target_extent_param)

        target_extent_crs_param = QgsProcessingParameterCrs(self.TARGET_EXTENT_CRS,
                                                            self.tr('CRS of the target raster extent'),
                                                            optional=True)
        target_extent_crs_param.setFlags(target_extent_crs_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(target_extent_crs_param)

        multithreading_param = QgsProcessingParameterBoolean(self.MULTITHREADING,
                                                             self.tr('Use multithreaded warping implementation'),
                                                             defaultValue=False)
        multithreading_param.setFlags(multithreading_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(multithreading_param)

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
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
                                                    defaultValue=0)
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

    def groupId(self):
        return 'rasterextraction'

    def commandName(self):
        return 'gdalwarp'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        maskLayer, maskLayerName = self.getOgrCompatibleSource(self.MASK, parameters, context, feedback, executing)

        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        arguments = []

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            arguments.append('-ot ' + self.TYPES[data_type])

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

        if nodata is not None:
            arguments.append('-dstnodata {}'.format(nodata))

        extent = self.parameterAsExtent(parameters, self.TARGET_EXTENT, context)
        if not extent.isNull():
            arguments.append('-te')
            arguments.append(extent.xMinimum())
            arguments.append(extent.yMinimum())
            arguments.append(extent.xMaximum())
            arguments.append(extent.yMaximum())

            extentCrs = self.parameterAsCrs(parameters, self.TARGET_EXTENT_CRS, context)
            if extentCrs:
                arguments.append('-te_srs')
                arguments.append(extentCrs.authid())

        if self.parameterAsBool(parameters, self.MULTITHREADING, context):
            arguments.append('-multi')

        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
