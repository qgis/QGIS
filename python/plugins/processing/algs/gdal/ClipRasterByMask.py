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

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
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
    SOURCE_CRS = 'SOURCE_CRS'
    TARGET_CRS = 'TARGET_CRS'
    EXTENT = 'TARGET_EXTENT'
    NODATA = 'NODATA'
    ALPHA_BAND = 'ALPHA_BAND'
    CROP_TO_CUTLINE = 'CROP_TO_CUTLINE'
    KEEP_RESOLUTION = 'KEEP_RESOLUTION'
    SET_RESOLUTION = 'SET_RESOLUTION'
    X_RESOLUTION = 'X_RESOLUTION'
    Y_RESOLUTION = 'Y_RESOLUTION'
    OPTIONS = 'OPTIONS'
    DATA_TYPE = 'DATA_TYPE'
    MULTITHREADING = 'MULTITHREADING'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [self.tr('Use Input Layer Data Type'), 'Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.MASK,
                                                              self.tr('Mask layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterCrs(self.SOURCE_CRS,
                                                    self.tr('Source CRS'),
                                                    optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.TARGET_CRS,
                                                    self.tr('Target CRS'),
                                                    optional=True))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Target extent'),
                                                       optional=True))
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
                                                        self.tr('Keep resolution of input raster'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.SET_RESOLUTION,
                                                        self.tr('Set output file resolution'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterNumber(self.X_RESOLUTION,
                                                       self.tr('X Resolution to output bands'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=None,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.Y_RESOLUTION,
                                                       self.tr('Y Resolution to output bands'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=None,
                                                       optional=True))

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

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(extra_param)

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

        sourceCrs = self.parameterAsCrs(parameters, self.SOURCE_CRS, context)
        targetCrs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)

        bbox = self.parameterAsExtent(parameters, self.EXTENT, context)
        bboxCrs = self.parameterAsExtentCrs(parameters, self.EXTENT, context)

        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = ['-overwrite']

        if sourceCrs.isValid():
            arguments.append('-s_srs')
            arguments.append(GdalUtils.gdal_crs_string(sourceCrs))

        if targetCrs.isValid():
            arguments.append('-t_srs')
            arguments.append(GdalUtils.gdal_crs_string(targetCrs))

        if not bbox.isNull():
            arguments.append('-te')
            arguments.append(str(bbox.xMinimum()))
            arguments.append(str(bbox.yMinimum()))
            arguments.append(str(bbox.xMaximum()))
            arguments.append(str(bbox.yMaximum()))
            arguments.append('-te_srs')
            arguments.append(GdalUtils.gdal_crs_string(bboxCrs))

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            arguments.append('-ot ' + self.TYPES[data_type])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        if self.parameterAsBoolean(parameters, self.KEEP_RESOLUTION, context):
            arguments.append('-tr')
            arguments.append(str(inLayer.rasterUnitsPerPixelX()))
            arguments.append(str(-inLayer.rasterUnitsPerPixelY()))
            arguments.append('-tap')

        if self.parameterAsBoolean(parameters, self.SET_RESOLUTION, context):
            arguments.append('-tr')
            if self.X_RESOLUTION in parameters and parameters[self.X_RESOLUTION] is not None:
                xres = self.parameterAsDouble(parameters, self.X_RESOLUTION, context)
                arguments.append('{}'.format(xres))
            else:
                arguments.append(str(inLayer.rasterUnitsPerPixelX()))
            if self.Y_RESOLUTION in parameters and parameters[self.Y_RESOLUTION] is not None:
                yres = self.parameterAsDouble(parameters, self.Y_RESOLUTION, context)
                arguments.append('{}'.format(yres))
            else:
                arguments.append(str(-inLayer.rasterUnitsPerPixelY()))
            arguments.append('-tap')

        arguments.append('-cutline')
        arguments.append(maskLayer)
        arguments.append('-cl')
        arguments.append(maskLayerName)

        if self.parameterAsBoolean(parameters, self.CROP_TO_CUTLINE, context):
            arguments.append('-crop_to_cutline')

        if self.parameterAsBoolean(parameters, self.ALPHA_BAND, context):
            arguments.append('-dstalpha')

        if nodata is not None:
            arguments.append('-dstnodata {}'.format(nodata))

        if self.parameterAsBoolean(parameters, self.MULTITHREADING, context):
            arguments.append('-multi')

        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
