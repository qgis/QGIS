# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgsgdalcompress.py
    ---------------------
    Date                 : October 2020
    Copyright            : (C) 2020 by Alex RL
    Email                : roya0045 at github dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alex RL'
__date__ = 'October 2020'
__copyright__ = '(C) 2020, Alex RL'

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
    COMPRESSION = 'COMPRESSION'
    NUM_THREADS = 'NUM_THREADS'
    PREDICTOR = 'PREDICTOR'
    SPARSE = 'SPARSE'
    QUALITY = 'QUALITY'
    JPEGTABLESMODE = 'JPEGTABLESMODE'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [self.tr('Use Input Layer Data Type'), 'Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']
		self.COMPRESSOR = ["JPEG","LZW","PACKBITS","DEFLATE","LZMA","ZSTD","LERC","LERC_DEFLATE","LERC_ZSTD","WEBP","CCITTRLE","CCITTFAX3","CCITTFAX4","NONE"]
		
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsPRocessingParameterEnum(self.COMPRESSOR,
                                                     self.tr('Compression method to use'),
                                                     self.COMPRESSION,
                                                     allowMultiple=False,
                                                     defaultValue=3))

        self.addParameter(QgsProcessingParameterNumber(self.NUM_THREADS,
                                                       self.tr('Number of thread to use for compression'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       defaultValue=-1,minValue=-1,maxValue = (int)(os.popen('grep -c cores /proc/cpuinfo').read()),
                                                       optional=True))

        self.addParameter(QgsProcessingParameterNumber(self.PREDICTOR,
                                                       self.tr('Sets the LZW, DEFLATE and ZSTD compression.'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       defaultValue=1,minValue=1,maxValue =3,
                                                       optional=True))

        sparse=QgsProcessingParameterBoolean(self.SPARSE,
                                             self.tr('Make output sparse (save space, possibly not supported by other reader)'),
                                             defaultValue=False)
        sparse.setFlags(sparse.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(sparse)

        self.addParameter(QgsProcessingParameterNumber(self.QUALITY,
                                                       self.tr('Control Quality/compression, 100 is best quality, 1 is most compression, default is 75'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       defaultValue=75,minValue=1,maxValue = 100,
                                                       optional=True))

        tables=QgsProcessingParameterNumber(self.JPEGTABLESMODE,
                                                       self.tr('Configure JPEG quantisation'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       defaultValue=0,minValue=0,maxValue = 3,
                                                       optional=True)
        tables.setFlags(tables.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(tables)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=0)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

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

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Compressed')))

    def name(self):
        return 'compress'

    def displayName(self):
        return self.tr('Creates a compressed tiff of a raster')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'translate.png'))

    def commandName(self):
        return 'gdal_compress'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        qualArg = None
        pred=False

        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = []
        arguments.append('-of GTiff')

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            arguments.append('-ot ' + self.TYPES[data_type])

        arguments.append('-strict')


        predictor = self.parameterAsInteger( parameters, self.PREDICTOR,context)    

        compression = self.COMPRESSOR[self.parameterAsEnum(parameters, self.COMPRESSION,context)]
        arguments.append('-co '+'{}={}'.format('COMPRESS',compression))

        quality = self.parameterAsInteger( parameters, self.QUALITY,context)/100.0

		if 'DEFLATE' in compression :
			qualArg='-ZLEVEL'
            if compression == 'LERC_DEFLATE':
                quality = int(quality*12)
            else:
                quality = int(quality*9)
            pred = True
		elif compression =='JPG':
			quality=int(quality*100)
            qualArg='-JPEG_QUALITY'
            jpgtables=self.parameterAsInteger(parameters,self.JPEGTABLESMODE,context)
            arguments.append('-co '+'{}={}'.format('JPEGTABLESMODE',jpgtables))
		elif compression =='WEBP':
			qualArg = '-WEBP_LEVEL'
            quality=int(quality*100)
		elif 'ZSTD' in compression:
			qualArg='-ZSTD_LEVEL'
            quality = int(quality*22)
            pred=True
		elif compression = 'LZW':
			pred=True

        if qualArg:
            arguments.append('-co '+'{}={}'.format(qualArg,max(quality,1))
        
        if pred:
            arguments.append('-co '+'{}={}'.format('PREDICTOR',predictor)

        threads=self.parameterAsInteger(parameters,self.NUM_THREADS,context)
        arguments.append('-co '+'{}={}'.format('NUM_THREADS',threads))

        if self.parameterAsBoolean(parameters, self.SPARSE, context):
            arguments.append('-co '+'{}={}'.format('SPARSE_OK','TRUE'))


        arguments.append('-ot ' +'BIGTIFF=IF_SAFER')

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
