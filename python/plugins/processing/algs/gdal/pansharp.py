# -*- coding: utf-8 -*-

"""
***************************************************************************
    pansharp.py
    ---------------------
    Date                 : March 2019
    Copyright            : (C) 2019 by Alexander Bruy
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
__date__ = 'March 2019'
__copyright__ = '(C) 2019, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class pansharp(GdalAlgorithm):

    SPECTRAL = 'SPECTRAL'
    PANCHROMATIC = 'PANCHROMATIC'
    RESAMPLING = 'RESAMPLING'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = ((self.tr('Nearest neighbour'), 'nearest'),
                        (self.tr('Bilinear'), 'bilinear'),
                        (self.tr('Cubic'), 'cubic'),
                        (self.tr('Cubic spline'), 'cubicspline'),
                        (self.tr('Lanczos windowed sinc'), 'lanczos'),
                        (self.tr('Average'), 'average'))

        self.addParameter(QgsProcessingParameterRasterLayer(self.SPECTRAL,
                                                            self.tr('Spectral dataset')))
        self.addParameter(QgsProcessingParameterRasterLayer(self.PANCHROMATIC,
                                                            self.tr('Panchromatic dataset')))

        resampling_param = QgsProcessingParameterEnum(self.RESAMPLING,
                                                      self.tr('Resampling algorithm'),
                                                      options=[i[0] for i in self.methods],
                                                      defaultValue=2)
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

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Output')))

    def name(self):
        return 'pansharp'

    def displayName(self):
        return self.tr('Pansharpening')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def commandName(self):
        return 'gdal_pansharpen'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        spectral = self.parameterAsRasterLayer(parameters, self.SPECTRAL, context)
        if spectral is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.SPECTRAL))

        panchromatic = self.parameterAsRasterLayer(parameters, self.PANCHROMATIC, context)
        if panchromatic is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.PANCHROMATIC))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = []
        arguments.append(panchromatic.source())
        arguments.append(spectral.source())
        arguments.append(out)

        arguments.append('-r')
        arguments.append(self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1])
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if isWindows():
            commands = ['python3', '-m', self.commandName()]
        else:
            commands = [self.commandName() + '.py']

        commands.append(GdalUtils.escapeAndJoin(arguments))

        return commands
