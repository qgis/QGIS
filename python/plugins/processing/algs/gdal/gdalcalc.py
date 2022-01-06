# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdalcalc.py
    ---------------------
    Date                 : Janaury 2015
    Copyright            : (C) 2015 by Giovanni Manghi
    Email                : giovanni dot manghi at naturalgis dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giovanni Manghi'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Giovanni Manghi'

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows


class gdalcalc(GdalAlgorithm):
    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'
    INPUT_C = 'INPUT_C'
    INPUT_D = 'INPUT_D'
    INPUT_E = 'INPUT_E'
    INPUT_F = 'INPUT_F'
    BAND_A = 'BAND_A'
    BAND_B = 'BAND_B'
    BAND_C = 'BAND_C'
    BAND_D = 'BAND_D'
    BAND_E = 'BAND_E'
    BAND_F = 'BAND_F'
    FORMULA = 'FORMULA'
    EXTENT = 'PROJWIN'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    RTYPE = 'RTYPE'
    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_A,
                self.tr('Input layer A'),
                optional=False))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_A,
                self.tr('Number of raster band for A'),
                parentLayerParameterName=self.INPUT_A))
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_B,
                self.tr('Input layer B'),
                optional=True))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_B,
                self.tr('Number of raster band for B'),
                parentLayerParameterName=self.INPUT_B,
                optional=True))
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_C,
                self.tr('Input layer C'),
                optional=True))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_C,
                self.tr('Number of raster band for C'),
                parentLayerParameterName=self.INPUT_C,
                optional=True))
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_D,
                self.tr('Input layer D'),
                optional=True))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_D,
                self.tr('Number of raster band for D'),
                parentLayerParameterName=self.INPUT_D,
                optional=True))
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_E,
                self.tr('Input layer E'),
                optional=True))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_E,
                self.tr('Number of raster band for E'),
                parentLayerParameterName=self.INPUT_E,
                optional=True))
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.INPUT_F,
                self.tr('Input layer F'),
                optional=True))
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND_F,
                self.tr('Number of raster band for F'),
                parentLayerParameterName=self.INPUT_F,
                optional=True))
        self.addParameter(
            QgsProcessingParameterString(
                self.FORMULA,
                self.tr('Calculation in gdalnumeric syntax using +-/* or any numpy array functions (i.e. logical_and())'),
                'A*2',
                optional=False))
        self.addParameter(
            QgsProcessingParameterNumber(
                self.NO_DATA,
                self.tr('Set output nodata value'),
                type=QgsProcessingParameterNumber.Double,
                defaultValue=None,
                optional=True))

        if GdalUtils.version() >= 3030000:
            extent_param = QgsProcessingParameterExtent(self.EXTENT,
                                                        self.tr('Output extent'),
                                                        optional=True)
            extent_param.setHelp(self.tr('Custom extent of the output raster'))
            self.addParameter(extent_param)

        self.addParameter(
            QgsProcessingParameterEnum(
                self.RTYPE,
                self.tr('Output raster type'),
                options=self.TYPE,
                defaultValue=5))

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

        self.addParameter(
            QgsProcessingParameterRasterDestination(
                self.OUTPUT,
                self.tr('Calculated')))

    def name(self):
        return 'rastercalculator'

    def displayName(self):
        return self.tr('Raster calculator')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def commandName(self):
        return 'gdal_calc'

    def processAlgorithm(self, parameters, context, feedback):
        if GdalUtils.version() < 3030000 and self.EXTENT in parameters.keys():
            raise QgsProcessingException(self.tr('The output extent option is only available on GDAL 3.3 or later'))
        return GdalAlgorithm.processAlgorithm(self, parameters, context, feedback)

    def getConsoleCommands(self, parameters, context, feedback, executing=True):

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        formula = self.parameterAsString(parameters, self.FORMULA, context)
        if self.NO_DATA in parameters and parameters[self.NO_DATA] is not None:
            noData = self.parameterAsDouble(parameters, self.NO_DATA, context)
        else:
            noData = None

        arguments = [
            '--overwrite',
            f'--calc "{formula}"',
            '--format',
            GdalUtils.getFormatShortNameFromFilename(out),
            '--type',
            self.TYPE[self.parameterAsEnum(parameters, self.RTYPE, context)]
        ]

        if noData is not None:
            arguments.append('--NoDataValue')
            arguments.append(noData)
        layer = self.parameterAsRasterLayer(parameters, self.INPUT_A, context)
        if layer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_A))

        bbox = self.parameterAsExtent(parameters, self.EXTENT, context, layer.crs())
        if not bbox.isNull():
            arguments.append('--projwin')
            arguments.append(str(bbox.xMinimum()))
            arguments.append(str(bbox.yMaximum()))
            arguments.append(str(bbox.xMaximum()))
            arguments.append(str(bbox.yMinimum()))

        arguments.append('-A')
        arguments.append(layer.source())
        if self.parameterAsString(parameters, self.BAND_A, context):
            arguments.append('--A_band ' + self.parameterAsString(parameters, self.BAND_A, context))

        if self.INPUT_B in parameters and parameters[self.INPUT_B] is not None:
            layer = self.parameterAsRasterLayer(parameters, self.INPUT_B, context)
            if layer is None:
                raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_B))
            arguments.append('-B')
            arguments.append(layer.source())
            if self.parameterAsString(parameters, self.BAND_B, context):
                arguments.append('--B_band ' + self.parameterAsString(parameters, self.BAND_B, context))

        if self.INPUT_C in parameters and parameters[self.INPUT_C] is not None:
            layer = self.parameterAsRasterLayer(parameters, self.INPUT_C, context)
            if layer is None:
                raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_C))
            arguments.append('-C')
            arguments.append(layer.source())
            if self.parameterAsString(parameters, self.BAND_C, context):
                arguments.append('--C_band ' + self.parameterAsString(parameters, self.BAND_C, context))

        if self.INPUT_D in parameters and parameters[self.INPUT_D] is not None:
            layer = self.parameterAsRasterLayer(parameters, self.INPUT_D, context)
            if layer is None:
                raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_D))
            arguments.append('-D')
            arguments.append(layer.source())
            if self.parameterAsString(parameters, self.BAND_D, context):
                arguments.append('--D_band ' + self.parameterAsString(parameters, self.BAND_D, context))

        if self.INPUT_E in parameters and parameters[self.INPUT_E] is not None:
            layer = self.parameterAsRasterLayer(parameters, self.INPUT_E, context)
            if layer is None:
                raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_E))
            arguments.append('-E')
            arguments.append(layer.source())
            if self.parameterAsString(parameters, self.BAND_E, context):
                arguments.append('--E_band ' + self.parameterAsString(parameters, self.BAND_E, context))

        if self.INPUT_F in parameters and parameters[self.INPUT_F] is not None:
            layer = self.parameterAsRasterLayer(parameters, self.INPUT_F, context)
            if layer is None:
                raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT_F))
            arguments.append('-F')
            arguments.append(layer.source())
            if self.parameterAsString(parameters, self.BAND_F, context):
                arguments.append('--F_band ' + self.parameterAsString(parameters, self.BAND_F, context))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            parts = options.split('|')
            for p in parts:
                arguments.append('--co ' + p)

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append('--outfile')
        arguments.append(out)

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]
