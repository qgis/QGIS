"""
***************************************************************************
    rearrange_bands.py
    ---------------------
    Date                 : August 2018
    Copyright            : (C) 2018 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Mathieu Pellerin"
__date__ = "August 2018"
__copyright__ = "(C) 2018, Mathieu Pellerin"

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterEnum,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterBand,
    QgsProcessingParameterString,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rearrange_bands(GdalAlgorithm):
    INPUT = "INPUT"
    BANDS = "BANDS"
    OPTIONS = "OPTIONS"
    DATA_TYPE = "DATA_TYPE"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.TYPES = [
            self.tr("Use Input Layer Data Type"),
            "Byte",
            "Int16",
            "UInt16",
            "UInt32",
            "Int32",
            "Float32",
            "Float64",
            "CInt16",
            "CInt32",
            "CFloat32",
            "CFloat64",
            "Int8",
        ]

        self.addParameter(
            QgsProcessingParameterRasterLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterBand(
                self.BANDS,
                self.tr("Selected band(s)"),
                None,
                self.INPUT,
                allowMultiple=True,
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

        dataType_param = QgsProcessingParameterEnum(
            self.DATA_TYPE,
            self.tr("Output data type"),
            self.TYPES,
            allowMultiple=False,
            defaultValue=0,
        )
        dataType_param.setFlags(
            dataType_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(dataType_param)

        self.addParameter(
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Converted"))
        )

    def name(self):
        return "rearrange_bands"

    def displayName(self):
        return self.tr("Rearrange bands")

    def group(self):
        return self.tr("Raster conversion")

    def groupId(self):
        return "rasterconversion"

    def icon(self):
        return QIcon(os.path.join(pluginPath, "images", "gdaltools", "translate.png"))

    def shortHelpString(self):
        return self.tr(
            "This algorithm creates a new raster using selected band(s) from a given raster layer.\n\n"
            "The algorithm also makes it possible to reorder the bands for the newly-created raster."
        )

    def commandName(self):
        return "gdal_translate"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(inLayer)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        bands = self.parameterAsInts(parameters, self.BANDS, context)
        arguments = [f"-b {band}" for band in bands]

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            if self.TYPES[data_type] == "Int8" and GdalUtils.version() < 3070000:
                raise QgsProcessingException(
                    self.tr("Int8 data type requires GDAL version 3.7 or later")
                )

            arguments.append("-ot " + self.TYPES[data_type])

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments.append("-of")
        arguments.append(output_format)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        arguments.append(input_details.connection_string)
        arguments.append(out)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
