"""
***************************************************************************
    ogr2ogr.py
    ---------------------
    Date                 : November 2012
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

__author__ = "Victor Olaya"
__date__ = "November 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os

from qgis.core import (
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterString,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ogr2ogr(GdalAlgorithm):
    INPUT = "INPUT"
    CONVERT_ALL_LAYERS = "CONVERT_ALL_LAYERS"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Input layer"),
                types=[QgsProcessing.SourceType.TypeVector],
            )
        )

        convert_all_layers_param = QgsProcessingParameterBoolean(
            self.CONVERT_ALL_LAYERS,
            self.tr("Convert all layers from dataset"),
            defaultValue=False,
        )
        convert_all_layers_param.setHelp(
            self.tr(
                "Use convert all layers to convert a whole dataset. "
                "Supported output formats for this option are GPKG and GML."
            )
        )
        self.addParameter(convert_all_layers_param)

        options_param = QgsProcessingParameterString(
            self.OPTIONS,
            self.tr("Additional creation options"),
            defaultValue="",
            optional=True,
        )
        options_param.setFlags(
            options_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(options_param)

        self.addParameter(
            QgsProcessingParameterVectorDestination(self.OUTPUT, self.tr("Converted"))
        )

    def name(self):
        return "convertformat"

    def displayName(self):
        return self.tr("Convert format")

    def group(self):
        return self.tr("Vector conversion")

    def groupId(self):
        return "vectorconversion"

    def commandName(self):
        return "ogr2ogr"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        convertAllLayers = self.parameterAsBoolean(
            parameters, self.CONVERT_ALL_LAYERS, context
        )
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        if output_details.format in ("SQLite", "GPKG") and os.path.isfile(
            output_details.connection_string
        ):
            raise QgsProcessingException(
                self.tr('Output file "{}" already exists.').format(
                    output_details.connection_string
                )
            )

        arguments = []
        if output_details.format:
            arguments.append(f"-f {output_details.format}")

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if options:
            arguments.append(options)

        arguments.append(output_details.connection_string)
        arguments.append(input_details.connection_string)
        if not convertAllLayers:
            arguments.append(input_details.layer_name)

        return ["ogr2ogr", GdalUtils.escapeAndJoin(arguments)]

    def shortHelpString(self):
        return self.tr(
            "The algorithm converts simple features data between file formats.\n\n"
            "Use convert all layers to convert a whole dataset.\n"
            "Supported output formats for this option are:\n"
            "- GPKG\n"
            "- GML"
        )
