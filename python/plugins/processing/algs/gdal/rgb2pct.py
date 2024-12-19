"""
***************************************************************************
    rgb2pct.py
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
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterNumber,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rgb2pct(GdalAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NCOLORS = "NCOLORS"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterRasterLayer(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.NCOLORS,
                self.tr("Number of colors"),
                type=QgsProcessingParameterNumber.Type.Integer,
                minValue=0,
                maxValue=255,
                defaultValue=2,
            )
        )

        self.addParameter(
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("RGB to PCT"))
        )

    def name(self):
        return "rgbtopct"

    def displayName(self):
        return self.tr("RGB to PCT")

    def group(self):
        return self.tr("Raster conversion")

    def groupId(self):
        return "rasterconversion"

    def icon(self):
        return QIcon(
            os.path.join(pluginPath, "images", "gdaltools", "24-to-8-bits.png")
        )

    def commandName(self):
        return "rgb2pct"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(raster)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments = [
            "-n",
            str(self.parameterAsInt(parameters, self.NCOLORS, context)),
            "-of",
            output_format,
            input_details.connection_string,
            out,
        ]

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [
            self.commandName() + (".bat" if isWindows() else ".py"),
            GdalUtils.escapeAndJoin(arguments),
        ]
