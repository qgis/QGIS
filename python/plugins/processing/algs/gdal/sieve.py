"""
***************************************************************************
    sieve.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterNumber,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterString,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class sieve(GdalAlgorithm):
    INPUT = "INPUT"
    THRESHOLD = "THRESHOLD"
    EIGHT_CONNECTEDNESS = "EIGHT_CONNECTEDNESS"
    NO_MASK = "NO_MASK"
    MASK_LAYER = "MASK_LAYER"
    EXTRA = "EXTRA"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterRasterLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.THRESHOLD,
                self.tr("Threshold"),
                type=QgsProcessingParameterNumber.Type.Integer,
                minValue=0,
                defaultValue=10,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.EIGHT_CONNECTEDNESS,
                self.tr("Use 8-connectedness"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.NO_MASK,
                self.tr("Do not use the default validity mask for the input band"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.MASK_LAYER, self.tr("Validity mask"), optional=True
            )
        )

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
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Sieved"))
        )

    def name(self):
        return "sieve"

    def displayName(self):
        return self.tr("Sieve")

    def group(self):
        return self.tr("Raster analysis")

    def groupId(self):
        return "rasteranalysis"

    def icon(self):
        return QIcon(os.path.join(pluginPath, "images", "gdaltools", "sieve.png"))

    def commandName(self):
        return "gdal_sieve"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = [
            "-st",
            str(self.parameterAsInt(parameters, self.THRESHOLD, context)),
        ]

        if self.parameterAsBoolean(parameters, self.EIGHT_CONNECTEDNESS, context):
            arguments.append("-8")
        else:
            arguments.append("-4")

        if self.parameterAsBoolean(parameters, self.NO_MASK, context):
            arguments.append("-nomask")

        mask = self.parameterAsRasterLayer(parameters, self.MASK_LAYER, context)
        if mask:
            mask_details = GdalUtils.gdal_connection_details_from_layer(mask)
            arguments.append("-mask")
            arguments.append(mask_details.connection_string)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments.append("-of")
        arguments.append(output_format)

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(raster)

        arguments.append(input_details.connection_string)
        arguments.append(out)

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [
            self.commandName() + (".bat" if isWindows() else ".py"),
            GdalUtils.escapeAndJoin(arguments),
        ]
