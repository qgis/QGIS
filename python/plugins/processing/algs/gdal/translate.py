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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class translate(GdalAlgorithm):

    INPUT = 'INPUT'
    TARGET_CRS = 'TARGET_CRS'
    NODATA = 'NODATA'
    OUTSIZE = 'OUTSIZE'
    RESAMPLING_ALGORITHM = 'RESAMPLING_ALGORITHM'
    COPY_SUBDATASETS = 'COPY_SUBDATASETS'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [self.tr('Use Input Layer Data Type'), 'Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']
        self.resampling = ((self.tr('Nearest Neighbour'), 'nearest'),
                           (self.tr('Bilinear'), 'bilinear'),
                           (self.tr('Cubic Convolution'), 'cubic'),
                           (self.tr('B-Spline Convolution'), 'cubicspline'),
                           (self.tr('Lanczos Windowed Sinc'), 'lanczos'),
                           (self.tr('Average'), 'average'),
                           (self.tr('Mode'), 'mode'))

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterCrs(self.TARGET_CRS,
                                                    self.tr('Override the projection for the output file'),
                                                    defaultValue=None,
                                                    optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('Assign a specified nodata value to output bands'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=None,
                                                       optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.COPY_SUBDATASETS,
                                                        self.tr('Copy all subdatasets of this file to individual output files'),
                                                        defaultValue=False))

        outsize_param = QgsProcessingParameterString(self.OUTSIZE,
                                                     self.tr('X and Y size of output. Pixel size unless ‘%’ is appended, then will be a fraction of original size. One 0 value will be estimated to preserve aspect ratio.'),
                                                     defaultValue='100% 100%')
        outsize_param.setFlags(outsize_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(outsize_param)

        resampling_param = QgsProcessingParameterEnum(self.RESAMPLING_ALGORITHM,
                                                      self.tr('Resampling method to use, if output size is changed'),
                                                      options=[i[0] for i in self.resampling],
                                                      allowMultiple=False,
                                                      defaultValue=0)
        resampling_param.setFlags(resampling_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(resampling_param)

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        self.addParameter(options_param)

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(extra_param)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=0)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Converted')))

    def name(self):
        return 'translate'

    def displayName(self):
        return self.tr('Translate (convert format)')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'translate.png'))

    def commandName(self):
        return 'gdal_translate'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None

        arguments = []

        crs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)
        if crs.isValid():
            arguments.append('-a_srs')
            arguments.append(GdalUtils.gdal_crs_string(crs))

        outsize = self.parameterAsString(parameters, self.OUTSIZE, context)
        if not(len(outsize.split(' ')) < 2 or outsize == '100% 100%'):
            arguments.append('-outsize')
            arguments += outsize.strip().split(' ')[:2]
            arguments.append('-r')
            arguments.append(self.resampling[self.parameterAsEnum(parameters, self.RESAMPLING_ALGORITHM, context)][1])

        if nodata is not None:
            arguments.append('-a_nodata')
            arguments.append(nodata)

        if self.parameterAsBoolean(parameters, self.COPY_SUBDATASETS, context):
            arguments.append('-sds')

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            arguments.append('-ot ' + self.TYPES[data_type])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
