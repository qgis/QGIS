# -*- coding: utf-8 -*-

"""
***************************************************************************
    hillshade.py
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

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class hillshade(GdalAlgorithm):
    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    ZEVENBERGEN = 'ZEVENBERGEN'
    Z_FACTOR = 'Z_FACTOR'
    SCALE = 'SCALE'
    AZIMUTH = 'AZIMUTH'
    ALTITUDE = 'ALTITUDE'
    COMBINED = 'COMBINED'
    MULTIDIRECTIONAL = 'MULTIDIRECTIONAL'
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
        self.addParameter(QgsProcessingParameterNumber(self.Z_FACTOR,
                                                       self.tr('Z factor (vertical exaggeration)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.SCALE,
                                                       self.tr('Scale (ratio of vertical units to horizontal)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterNumber(self.AZIMUTH,
                                                       self.tr('Azimuth of the light'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       maxValue=360,
                                                       defaultValue=315.0))
        self.addParameter(QgsProcessingParameterNumber(self.ALTITUDE,
                                                       self.tr('Altitude of the light'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=45.0))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.ZEVENBERGEN,
                                                        self.tr("Use Zevenbergen&Thorne formula instead of the Horn's one"),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.COMBINED,
                                                        self.tr("Combined shading"),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.MULTIDIRECTIONAL,
                                                        self.tr("Multidirectional shading"),
                                                        defaultValue=False))

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

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Hillshade')))

    def name(self):
        return 'hillshade'

    def displayName(self):
        return self.tr('Hillshade')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def commandName(self):
        return 'gdaldem'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = ['hillshade']
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        arguments.append(inLayer.source())

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append(out)

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))
        arguments.append('-z')
        arguments.append(str(self.parameterAsDouble(parameters, self.Z_FACTOR, context)))
        arguments.append('-s')
        arguments.append(str(self.parameterAsDouble(parameters, self.SCALE, context)))

        multidirectional = self.parameterAsBoolean(parameters, self.MULTIDIRECTIONAL, context)
        # azimuth and multidirectional are mutually exclusive
        if not multidirectional:
            arguments.append('-az')
            arguments.append(str(self.parameterAsDouble(parameters, self.AZIMUTH, context)))

        arguments.append('-alt')
        arguments.append(str(self.parameterAsDouble(parameters, self.ALTITUDE, context)))

        if self.parameterAsBoolean(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        if self.parameterAsBoolean(parameters, self.ZEVENBERGEN, context):
            arguments.append('-alg')
            arguments.append('ZevenbergenThorne')

        combined = self.parameterAsBoolean(parameters, self.COMBINED, context)
        if combined and not multidirectional:
            arguments.append('-combined')
        elif multidirectional and not combined:
            arguments.append('-multidirectional')
        elif multidirectional and combined:
            raise QgsProcessingException(self.tr('Options -multirectional and -combined are mutually exclusive.'))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
