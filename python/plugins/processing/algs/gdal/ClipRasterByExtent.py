# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipRasterByExtent.py
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
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ClipRasterByExtent(GdalAlgorithm):
    INPUT = 'INPUT'
    EXTENT = 'PROJWIN'
    OVERCRS = 'OVERCRS'
    NODATA = 'NODATA'
    OPTIONS = 'OPTIONS'
    DATA_TYPE = 'DATA_TYPE'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [self.tr('Use Input Layer Data Type'), 'Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Clipping extent')))
        self.addParameter(QgsProcessingParameterBoolean(self.OVERCRS,
                                                        self.tr('Override the projection for the output file'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('Assign a specified nodata value to output bands'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=None,
                                                       optional=True))

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
                                                                  self.tr('Clipped (extent)')))

    def name(self):
        return 'cliprasterbyextent'

    def displayName(self):
        return self.tr('Clip raster by extent')

    def group(self):
        return self.tr('Raster extraction')

    def groupId(self):
        return 'rasterextraction'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-clip.png'))

    def commandName(self):
        return "gdal_translate"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException('Invalid input layer {}'.format(parameters[self.INPUT] if self.INPUT in parameters else 'INPUT'))

        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, inLayer.crs())
        override_crs = self.parameterAsBoolean(parameters, self.OVERCRS, context)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = [
            '-projwin',
            str(bbox.xMinimum()),
            str(bbox.yMaximum()),
            str(bbox.xMaximum()),
            str(bbox.yMinimum()),
        ]

        crs = inLayer.crs()
        if override_crs and crs.isValid():
            arguments.append('-a_srs {}'.format(GdalUtils.gdal_crs_string(crs)))

        if nodata is not None:
            arguments.append('-a_nodata {}'.format(nodata))

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            arguments.append('-ot ' + self.TYPES[data_type])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
