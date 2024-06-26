"""
***************************************************************************
    slope.py
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
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class slope(GdalAlgorithm):
    INPUT = 'INPUT'
    BAND = 'BAND'
    AS_PERCENT = 'AS_PERCENT'
    SCALE = 'SCALE'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    ZEVENBERGEN = 'ZEVENBERGEN'
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
        self.addParameter(QgsProcessingParameterNumber(self.SCALE,
                                                       self.tr('Ratio of vertical units to horizontal'),
                                                       type=QgsProcessingParameterNumber.Type.Double,
                                                       minValue=0.0,
                                                       defaultValue=1.0))
        self.addParameter(QgsProcessingParameterBoolean(self.AS_PERCENT,
                                                        self.tr('Slope expressed as percent instead of degrees'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.ZEVENBERGEN,
                                                        self.tr("Use Zevenbergen&Thorne formula instead of the Horn's one"),
                                                        defaultValue=False))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        options_param.setMetadata({'widget_wrapper': {'widget_type': 'rasteroptions'}})
        self.addParameter(options_param)

        extra_param = QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True)
        extra_param.setFlags(extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced)
        self.addParameter(extra_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Slope')))

    def name(self):
        return 'slope'

    def displayName(self):
        return self.tr('Slope')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def commandName(self):
        return 'gdaldem'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))
        input_details = GdalUtils.gdal_connection_details_from_layer(
            inLayer)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr('Output format is invalid'))

        arguments = [
            'slope',
            input_details.connection_string,
            out,
            '-of',
            output_format,
            '-b',
            str(self.parameterAsInt(parameters, self.BAND, context)),
            '-s',
            str(self.parameterAsDouble(parameters, self.SCALE, context)),
        ]

        if self.parameterAsBoolean(parameters, self.AS_PERCENT, context):
            arguments.append('-p')

        if self.parameterAsBoolean(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        if self.parameterAsBoolean(parameters, self.ZEVENBERGEN, context):
            arguments.append('-alg')
            arguments.append('ZevenbergenThorne')

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
