"""
***************************************************************************
    fillnodata.py
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

from qgis.core import (
    QgsProcessingAlgorithm,
    QgsRasterFileWriter,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterBand,
    QgsProcessingParameterNumber,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterString,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class fillnodata(GdalAlgorithm):
    INPUT = "INPUT"
    BAND = "BAND"
    DISTANCE = "DISTANCE"
    ITERATIONS = "ITERATIONS"
    NO_MASK = "NO_MASK"
    MASK_LAYER = "MASK_LAYER"
    OPTIONS = "OPTIONS"
    EXTRA = "EXTRA"
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
            QgsProcessingParameterNumber(
                self.DISTANCE,
                self.tr(
                    "Maximum distance (in pixels) to search out for values to interpolate"
                ),
                type=QgsProcessingParameterNumber.Type.Integer,
                minValue=0,
                defaultValue=10,
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.ITERATIONS,
                self.tr(
                    "Number of smoothing iterations to run after the interpolation"
                ),
                type=QgsProcessingParameterNumber.Type.Integer,
                minValue=0,
                defaultValue=0,
            )
        )

        # The -nomask option is no longer supported since GDAL 3.4 and
        # it doesn't work as expected even using GDAL < 3.4 https://github.com/OSGeo/gdal/pull/4201
        nomask_param = QgsProcessingParameterBoolean(
            self.NO_MASK,
            self.tr("Do not use the default validity mask for the input band"),
            defaultValue=False,
            optional=True,
        )
        nomask_param.setFlags(
            nomask_param.flags() | QgsProcessingParameterDefinition.Flag.FlagHidden
        )
        self.addParameter(nomask_param)

        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.MASK_LAYER, self.tr("Validity mask"), optional=True
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
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Filled"))
        )

    def name(self):
        return "fillnodata"

    def displayName(self):
        return self.tr("Fill NoData")

    def group(self):
        return self.tr("Raster analysis")

    def groupId(self):
        return "rasteranalysis"

    def commandName(self):
        return "gdal_fillnodata"

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.Flag.FlagDisplayNameIsLiteral

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(raster)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        arguments = [
            input_details.connection_string,
            out,
            "-md",
            str(self.parameterAsInt(parameters, self.DISTANCE, context)),
        ]

        nIterations = self.parameterAsInt(parameters, self.ITERATIONS, context)
        if nIterations:
            arguments.append("-si")
            arguments.append(str(nIterations))

        arguments.append("-b")
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        mask = self.parameterAsRasterLayer(parameters, self.MASK_LAYER, context)
        if mask:
            arguments.append("-mask")
            arguments.append(mask.source())

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments.append("-of")
        arguments.append(output_format)

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        # Until https://github.com/OSGeo/gdal/issues/7651 is fixed, creation options should be latest argument
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        return [
            self.commandName() + (".bat" if isWindows() else ".py"),
            GdalUtils.escapeAndJoin(arguments),
        ]
