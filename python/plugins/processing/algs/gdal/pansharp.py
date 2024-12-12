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

__author__ = "Alexander Bruy"
__date__ = "March 2019"
__copyright__ = "(C) 2019, Alexander Bruy"

import os

from qgis.core import (
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterEnum,
    QgsProcessingParameterString,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class pansharp(GdalAlgorithm):
    SPECTRAL = "SPECTRAL"
    PANCHROMATIC = "PANCHROMATIC"
    RESAMPLING = "RESAMPLING"
    OPTIONS = "OPTIONS"
    EXTRA = "EXTRA"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = (
            (self.tr("Nearest Neighbour"), "nearest"),
            (self.tr("Bilinear (2x2 Kernel)"), "bilinear"),
            (self.tr("Cubic (4x4 Kernel)"), "cubic"),
            (self.tr("Cubic B-Spline (4x4 Kernel)"), "cubicspline"),
            (self.tr("Lanczos (6x6 Kernel)"), "lanczos"),
            (self.tr("Average"), "average"),
        )

        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.SPECTRAL, self.tr("Spectral dataset")
            )
        )
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.PANCHROMATIC, self.tr("Panchromatic dataset")
            )
        )

        resampling_param = QgsProcessingParameterEnum(
            self.RESAMPLING,
            self.tr("Resampling algorithm"),
            options=[i[0] for i in self.methods],
            defaultValue=2,
        )
        resampling_param.setFlags(
            resampling_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(resampling_param)

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

        extra_param = QgsProcessingParameterString(
            self.EXTRA,
            self.tr("Additional command-line parameters"),
            defaultValue=None,
            optional=True,
        )
        extra_param.setFlags(
            extra_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(extra_param)

        self.addParameter(
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Output"))
        )

    def name(self):
        return "pansharp"

    def displayName(self):
        return self.tr("Pansharpening")

    def group(self):
        return self.tr("Raster miscellaneous")

    def groupId(self):
        return "rastermiscellaneous"

    def commandName(self):
        return "gdal_pansharpen"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        spectral = self.parameterAsRasterLayer(parameters, self.SPECTRAL, context)
        if spectral is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.SPECTRAL)
            )
        spectral_input_details = GdalUtils.gdal_connection_details_from_layer(spectral)

        panchromatic = self.parameterAsRasterLayer(
            parameters, self.PANCHROMATIC, context
        )
        if panchromatic is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.PANCHROMATIC)
            )
        panchromatic_input_details = GdalUtils.gdal_connection_details_from_layer(
            panchromatic
        )

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments = [
            panchromatic_input_details.connection_string,
            spectral_input_details.connection_string,
            out,
            "-r",
            self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1],
            "-of",
            output_format,
        ]

        if panchromatic_input_details.credential_options:
            arguments.extend(
                panchromatic_input_details.credential_options_as_arguments()
            )

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        return [
            self.commandName() + (".bat" if isWindows() else ".py"),
            GdalUtils.escapeAndJoin(arguments),
        ]
