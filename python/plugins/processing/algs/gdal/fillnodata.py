# -*- coding: utf-8 -*-

"""
***************************************************************************
    fillnodata.py
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

from qgis.core import (QgsProcessingAlgorithm,
                       QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class fillnodata(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    DISTANCE = 'DISTANCE'
    ITERATIONS = 'ITERATIONS'
    NO_MASK = 'NO_MASK'
    MASK_LAYER = 'MASK_LAYER'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Maximum distance (in pixels) to search out for values to interpolate'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=10))
        self.addParameter(QgsProcessingParameterNumber(self.ITERATIONS,
                                                       self.tr('Number of smoothing iterations to run after the interpolation'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=0))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_MASK,
                                                        self.tr('Do not use the default validity mask for the input band'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterRasterLayer(self.MASK_LAYER,
                                                            self.tr('Validity mask'),
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

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(extra_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Filled')))

    def name(self):
        return 'fillnodata'

    def displayName(self):
        return self.tr('Fill nodata')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def commandName(self):
        return 'gdal_fillnodata'

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagDisplayNameIsLiteral

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []
        arguments.append('-md')
        arguments.append(str(self.parameterAsInt(parameters, self.DISTANCE, context)))

        nIterations = self.parameterAsInt(parameters, self.ITERATIONS, context)
        if nIterations:
            arguments.append('-si')
            arguments.append(str(nIterations))

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBoolean(parameters, self.NO_MASK, context):
            arguments.append('-nomask')

        mask = self.parameterAsRasterLayer(parameters, self.MASK_LAYER, context)
        if mask:
            arguments.append('-mask {}'.format(mask.source()))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(raster.source())
        arguments.append(out)

        if isWindows():
            commands = ["python3", "-m", self.commandName()]
        else:
            commands = [self.commandName() + '.py']

        commands.append(GdalUtils.escapeAndJoin(arguments))

        return commands
