# -*- coding: utf-8 -*-

"""
***************************************************************************
    GridInverseDistanceNearestNeighbor.py
    ---------------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Alexander Bruy
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


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


class GridInverseDistanceNearestNeighbor(GdalAlgorithm):

    INPUT = 'INPUT'
    Z_FIELD = 'Z_FIELD'
    POWER = 'POWER'
    SMOOTHING = 'SMOOTHING'
    RADIUS = 'RADIUS'
    MAX_POINTS = 'MAX_POINTS'
    MIN_POINTS = 'MIN_POINTS'
    NODATA = 'NODATA'
    OPTIONS = 'OPTIONS'
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
        self.addParameter(QgsProcessingParameterNumber(self.RADIUS,
                                                       self.tr('The radius of the search circle'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.MAX_POINTS,
                                                       self.tr('Maximum number of data points to use'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=12))
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

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Interpolated (IDW with NN search)')))

    def name(self):
        return 'gridinversedistancenearestneighbor'

    def displayName(self):
        return self.tr('Grid (IDW with nearest neighbor searching)')

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

        arguments = ['-l']
        arguments.append(layerName)

        fieldName = self.parameterAsString(parameters, self.Z_FIELD, context)
        if fieldName:
            arguments.append('-zfield')
            arguments.append(fieldName)

        params = 'invdistnn'
        params += ':power={}'.format(self.parameterAsDouble(parameters, self.POWER, context))
        params += ':smothing={}'.format(self.parameterAsDouble(parameters, self.SMOOTHING, context))
        params += ':radius={}'.format(self.parameterAsDouble(parameters, self.RADIUS, context))
        params += ':max_points={}'.format(self.parameterAsInt(parameters, self.MAX_POINTS, context))
        params += ':min_points={}'.format(self.parameterAsInt(parameters, self.MIN_POINTS, context))
        params += ':nodata={}'.format(self.parameterAsDouble(parameters, self.NODATA, context))

        arguments.append('-a')
        arguments.append(params)
        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        arguments.append(ogrLayer)
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
