# -*- coding: utf-8 -*-

"""
***************************************************************************
    viewshed.py
    ---------------------
    Date                 : October 2019
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
__date__ = 'October 2019'
__copyright__ = '(C) 2019, Alexander Bruy'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class viewshed(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    OBSERVER = 'OBSERVER'
    OBSERVER_HEIGHT = 'OBSERVER_HEIGHT'
    TARGET_HEIGHT = 'TARGET_HEIGHT'
    MAX_DISTANCE = 'MAX_DISTANCE'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterPoint(self.OBSERVER,
                                                      self.tr('Observer location')))
        self.addParameter(QgsProcessingParameterNumber(self.OBSERVER_HEIGHT,
                                                       self.tr('Observer height, DEM units'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.TARGET_HEIGHT,
                                                       self.tr('Target height, DEM units'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterDistance(self.MAX_DISTANCE,
                                                         self.tr('Maximum distance from observer to compute visibility'),
                                                         parentParameterName=self.INPUT,
                                                         minValue=0.0,
                                                         defaultValue=100.0))

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
                                                                  self.tr('Output')))

    def name(self):
        return 'viewshed'

    def displayName(self):
        return self.tr('Viewshed')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def commandName(self):
        return 'gdal_viewshed'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        dem = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if dem is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        observer = self.parameterAsPoint(parameters, self.OBSERVER, context, dem.crs())

        arguments = []
        arguments.append('-b')
        arguments.append('{}'.format(self.parameterAsInt(parameters, self.BAND, context)))
        arguments.append('-ox')
        arguments.append('{}'.format(observer.x()))
        arguments.append('-oy')
        arguments.append('{}'.format(observer.y()))
        arguments.append('-oz')
        arguments.append('{}'.format(self.parameterAsDouble(parameters, self.OBSERVER_HEIGHT, context)))
        arguments.append('-tz')
        arguments.append('{}'.format(self.parameterAsDouble(parameters, self.TARGET_HEIGHT, context)))
        arguments.append('-md')
        arguments.append('{}'.format(self.parameterAsDouble(parameters, self.MAX_DISTANCE, context)))

        arguments.append('-f')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(dem.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
