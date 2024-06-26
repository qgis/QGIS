"""
***************************************************************************
    ColorRelief.py
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
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ColorRelief(GdalAlgorithm):
    INPUT = 'INPUT'
    BAND = 'BAND'
    COMPUTE_EDGES = 'COMPUTE_EDGES'
    COLOR_TABLE = 'COLOR_TABLE'
    MATCH_MODE = 'MATCH_MODE'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.modes = ((self.tr('Use strict color matching'), '-exact_color_entry'),
                      (self.tr('Use closest RGBA quadruplet'), '-nearest_color_entry'),
                      (self.tr('Use smoothly blended colors'), ''))

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterBoolean(self.COMPUTE_EDGES,
                                                        self.tr('Compute edges'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterFile(self.COLOR_TABLE,
                                                     self.tr('Color configuration file')))
        self.addParameter(QgsProcessingParameterEnum(self.MATCH_MODE,
                                                     self.tr('Matching mode'),
                                                     options=[i[0] for i in self.modes],
                                                     defaultValue=2))

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

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Color relief')))

    def name(self):
        return 'colorrelief'

    def displayName(self):
        return self.tr('Color relief')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def commandName(self):
        return 'gdaldem'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = ['color-relief']
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))
        input_details = GdalUtils.gdal_connection_details_from_layer(inLayer)

        arguments.append(input_details.connection_string)
        arguments.append(self.parameterAsFile(parameters, self.COLOR_TABLE, context))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append(out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr('Output format is invalid'))

        arguments.append('-of')
        arguments.append(output_format)

        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if self.parameterAsBoolean(parameters, self.COMPUTE_EDGES, context):
            arguments.append('-compute_edges')

        mode = self.modes[self.parameterAsEnum(parameters, self.MATCH_MODE, context)][1]
        if mode != '':
            arguments.append(mode)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
