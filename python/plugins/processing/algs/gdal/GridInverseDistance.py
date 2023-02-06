# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridInverseDistance.py
    ---------------------
    Date                 : October 2013
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

__author__ = 'Alexander Bruy'
__date__ = 'October 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class GridInverseDistance(GdalAlgorithm):
    INPUT = 'INPUT'
    Z_FIELD = 'Z_FIELD'
    POWER = 'POWER'
    SMOOTHING = 'SMOOTHING'
    RADIUS_1 = 'RADIUS_1'
    RADIUS_2 = 'RADIUS_2'
    MAX_POINTS = 'MAX_POINTS'
    MIN_POINTS = 'MIN_POINTS'
    ANGLE = 'ANGLE'
    NODATA = 'NODATA'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Point layer'),
                                                              [QgsProcessing.TypeVectorPoint]))

        z_field_param = QgsProcessingParameterField(self.Z_FIELD,
                                                    self.tr('Z value from field'),
                                                    None,
                                                    self.INPUT,
                                                    QgsProcessingParameterField.Numeric,
                                                    optional=True)
        z_field_param.setFlags(z_field_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(z_field_param)

        self.addParameter(QgsProcessingParameterNumber(self.POWER,
                                                       self.tr('Weighting power'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       maxValue=100.0,
                                                       defaultValue=2.0))
        self.addParameter(QgsProcessingParameterNumber(self.SMOOTHING,
                                                       self.tr('Smoothing'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.RADIUS_1,
                                                       self.tr('The first radius of search ellipse'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.RADIUS_2,
                                                       self.tr('The second radius of search ellipse'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.ANGLE,
                                                       self.tr('Angle of search ellipse rotation in degrees (counter clockwise)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       maxValue=360.0,
                                                       defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.MAX_POINTS,
                                                       self.tr('Maximum number of data points to use'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_POINTS,
                                                       self.tr('Minimum number of data points to use'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.NODATA,
                                                       self.tr('NODATA marker to fill empty points'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.0))

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
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Interpolated (IDW)')))

    def name(self):
        return 'gridinversedistance'

    def displayName(self):
        return self.tr('Grid (Inverse distance to a power)')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'grid.png'))

    def commandName(self):
        return 'gdal_grid'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        ogrLayer, layerName = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)

        arguments = [
            '-l',
            layerName
        ]
        fieldName = self.parameterAsString(parameters, self.Z_FIELD, context)
        if fieldName:
            arguments.append('-zfield')
            arguments.append(fieldName)

        params = 'invdist'
        params += ':power={}'.format(self.parameterAsDouble(parameters, self.POWER, context))
        params += ':smoothing={}'.format(self.parameterAsDouble(parameters, self.SMOOTHING, context))
        params += ':radius1={}'.format(self.parameterAsDouble(parameters, self.RADIUS_1, context))
        params += ':radius2={}'.format(self.parameterAsDouble(parameters, self.RADIUS_2, context))
        params += ':angle={}'.format(self.parameterAsDouble(parameters, self.ANGLE, context))
        params += ':max_points={}'.format(self.parameterAsInt(parameters, self.MAX_POINTS, context))
        params += ':min_points={}'.format(self.parameterAsInt(parameters, self.MIN_POINTS, context))
        params += ':nodata={}'.format(self.parameterAsDouble(parameters, self.NODATA, context))

        arguments.append('-a')
        arguments.append(params)
        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(ogrLayer)
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
