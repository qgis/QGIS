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

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingOutputRasterLayer)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class gdaladdo(GdalAlgorithm):

    INPUT = 'INPUT'
    LEVELS = 'LEVELS'
    CLEAN = 'CLEAN'
    RESAMPLING = 'RESAMPLING'
    FORMAT = 'FORMAT'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = ((self.tr('Nearest Neighbour'), 'nearest'),
                        (self.tr('Average'), 'average'),
                        (self.tr('Gaussian'), 'gauss'),
                        (self.tr('Cubic Convolution'), 'cubic'),
                        (self.tr('B-Spline Convolution'), 'cubicspline'),
                        (self.tr('Lanczos Windowed Sinc'), 'lanczos'),
                        (self.tr('Average MP'), 'average_mp'),
                        (self.tr('Average in Mag/Phase Space'), 'average_magphase'),
                        (self.tr('Mode'), 'mode'))

        self.formats = (self.tr('Internal (if possible)'),
                        self.tr('External (GTiff .ovr)'),
                        self.tr('External (ERDAS Imagine .aux)'))

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterString(self.LEVELS,
                                                       self.tr('Overview levels'),
                                                       defaultValue='2 4 8 16'))
        self.addParameter(QgsProcessingParameterBoolean(self.CLEAN,
                                                        self.tr('Remove all existing overviews'),
                                                        defaultValue=False))

        params = []
        params.append(QgsProcessingParameterEnum(self.RESAMPLING,
                                                 self.tr('Resampling method'),
                                                 options=[i[0] for i in self.methods],
                                                 allowMultiple=False,
                                                 defaultValue=0,
                                                 optional=True))
        params.append(QgsProcessingParameterEnum(self.FORMAT,
                                                 self.tr('Overviews format'),
                                                 options=self.formats,
                                                 allowMultiple=False,
                                                 defaultValue=0,
                                                 optional=True))
        params.append(QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True))
        for p in params:
            p.setFlags(p.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(p)

        self.addOutput(QgsProcessingOutputRasterLayer(self.OUTPUT, self.tr('Pyramidized')))

    def name(self):
        return 'overviews'

    def displayName(self):
        return self.tr('Build overviews (pyramids)')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'raster-overview.png'))

    def commandName(self):
        return 'gdaladdo'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        fileName = inLayer.source()

        arguments = []
        arguments.append(fileName)

        arguments.append('-r')
        arguments.append(self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1])

        ovrFormat = self.parameterAsEnum(parameters, self.FORMAT, context)
        if ovrFormat == 1:
            arguments.append('-ro')
        elif ovrFormat == 2:
            arguments.extend('--config USE_RRD YES'.split(' '))

        if self.parameterAsBoolean(parameters, self.CLEAN, context):
            arguments.append('-clean')

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.extend(self.parameterAsString(parameters, self.LEVELS, context).split(' '))

        self.setOutputValue(self.OUTPUT, fileName)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
