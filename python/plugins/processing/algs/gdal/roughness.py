"""
***************************************************************************
    roughness.py
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

__author__ = "Alexander Bruy"
__date__ = "October 2013"
__copyright__ = "(C) 2013, Alexander Bruy"

import os

from qgis.core import (
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterBand,
    QgsProcessingParameterString,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterRasterDestination,
)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class roughness(GdalAlgorithm):
    INPUT = "INPUT"
    BAND = "BAND"
    COMPUTE_EDGES = "COMPUTE_EDGES"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterRasterLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterBand(
                self.BAND,
                self.tr("Band number"),
                1,
                parentLayerParameterName=self.INPUT,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.COMPUTE_EDGES, self.tr("Compute edges"), defaultValue=False
            )
        )

        options_param = QgsProcessingParameterString(
            self.OPTIONS,
            self.tr("Additional creation options"),
            defaultValue="",
            optional=True,
        )
        options_param.setFlags(
            options_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        options_param.setMetadata({"widget_wrapper": {"widget_type": "rasteroptions"}})
        self.addParameter(options_param)

        self.addParameter(
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Roughness"))
        )

    def name(self):
        return "roughness"

    def displayName(self):
        return self.tr("Roughness")

    def group(self):
        return self.tr("Raster analysis")

    def groupId(self):
        return "rasteranalysis"

    def commandName(self):
        return "gdaldem"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(inLayer)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments = [
            "roughness",
            input_details.connection_string,
            out,
            "-of",
            output_format,
            "-b",
            str(self.parameterAsInt(parameters, self.BAND, context)),
        ]

        if self.parameterAsBoolean(parameters, self.COMPUTE_EDGES, context):
            arguments.append("-compute_edges")

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
