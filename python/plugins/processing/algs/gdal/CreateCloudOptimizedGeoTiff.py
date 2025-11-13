"""
***************************************************************************
    create_cog.py
    ---------------------
    Date                 : October 2025
    Copyright            : (C) 2025 by Jan Caha
    Email                : jan.caha at outlook dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Jan Caha"
__date__ = "October 2025"
__copyright__ = "(C) 2025 by Jan Caha"

import os
from pathlib import Path

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingAlgorithm,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingMultiStepFeedback,
    QgsProcessingOutputMultipleLayers,
    QgsProcessingParameterFolderDestination,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterString,
    QgsRasterLayer,
)
from processing.algs.gdal.GdalUtils import GdalUtils


class CreateCloudOptimizedGeoTIFF(QgsProcessingAlgorithm):
    LAYERS = "LAYERS"
    CREATION_OPTIONS = "CREATION_OPTIONS"
    OUTPUT = "OUTPUT"
    OUTPUT_LAYERS = "OUTPUT_LAYERS"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.addParameter(
            QgsProcessingParameterMultipleLayers(
                self.LAYERS, self.tr("Input layers"), Qgis.ProcessingSourceType.Raster
            )
        )

        creation_options_param = QgsProcessingParameterString(
            self.CREATION_OPTIONS,
            self.tr("Additional creation options"),
            defaultValue="",
            optional=True,
        )
        creation_options_param.setFlags(
            creation_options_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        creation_options_param.setMetadata(
            {"widget_wrapper": {"widget_type": "rasteroptions"}}
        )
        self.addParameter(creation_options_param)

        self.addParameter(
            QgsProcessingParameterFolderDestination(
                self.OUTPUT, self.tr("Output directory")
            )
        )

        self.addOutput(
            QgsProcessingOutputMultipleLayers(
                self.OUTPUT_LAYERS, self.tr("Output layers")
            )
        )

    def name(self):
        return "createcog"

    def displayName(self):
        return self.tr("Create Cloud Optimized GeoTIFF")

    def group(self):
        return self.tr("Raster conversion")

    def groupId(self):
        return "rasterconversion"

    def shortHelpString(self):
        return self.tr(
            "This algorithm creates a Cloud Optimized GeoTIFF (COG) from the input raster layers."
        )

    def tags(self):
        return self.tr("gdal,gdal,gdal_translate,COG,cloud optimized geotiff").split(
            ","
        )

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGdal.svg")

    def createInstance(self):
        return self.__class__()

    def tr(self, string, context=""):
        if context == "":
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def commandName(self):
        return "gdal_translate"

    def output_path(self, input_layer: QgsRasterLayer) -> str:
        # if source is a file baseName is used for output, otherwise use layer name
        fileInfo = Path(input_layer.source())

        if fileInfo.is_file():
            output_file = fileInfo.stem + self.extension
        else:
            output_file = input_layer.name() + self.extension

        return os.path.join(self.output_dir, output_file)

    def getTranslateCommand(
        self, input_layer: QgsRasterLayer, output_path: str
    ) -> list[str]:
        """Builds the gdal_translate command for a given input layer.
        Returns a tuple of output file path and command arguments list."""

        arguments = self.common_arguments.copy()

        input_details = GdalUtils.gdal_connection_details_from_layer(input_layer)

        arguments.append(input_details.connection_string)

        arguments.append(output_path)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]

    def processAlgorithm(self, parameters, context, feedback):

        self.extension = ".tif"

        self.output_layers = []

        self.common_arguments = []

        self.common_arguments.append("-of")
        self.common_arguments.append("COG")

        options = self.parameterAsString(parameters, self.CREATION_OPTIONS, context)

        if options:
            self.common_arguments.extend(GdalUtils.parseCreationOptions(options))

        self.inputLayers = self.parameterAsLayerList(parameters, self.LAYERS, context)

        if not self.inputLayers:
            raise QgsProcessingException(self.tr("No layers selected"))

        self.output_dir = self.parameterAsString(parameters, self.OUTPUT, context)

        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

        feedback = QgsProcessingMultiStepFeedback(len(self.inputLayers), feedback)

        for i, inputLayer in enumerate(self.inputLayers):

            if feedback.isCanceled():
                return {}

            feedback.setCurrentStep(i)

            if inputLayer is None or not inputLayer.isValid():
                continue

            feedback.pushInfo(f"Processing layer: {inputLayer.name()}")

            output_file = self.output_path(inputLayer)

            command = self.getTranslateCommand(inputLayer, output_file)

            if Path(output_file).exists():
                feedback.pushWarning(
                    f"Output file {output_file} already exists. It will be overwritten."
                )

            GdalUtils.runGdal(command, feedback)

            if Path(output_file).exists():
                self.output_layers.append(output_file)

        return {
            self.OUTPUT: self.output_dir,
            self.OUTPUT_LAYERS: self.output_layers,
        }
