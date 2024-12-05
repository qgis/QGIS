"""
***************************************************************************
    merge.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Radoslaw Guzinski
    Email                : rmgu at dhi-gras dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Radoslaw Guzinski"
__date__ = "October 2014"
__copyright__ = "(C) 2014, Radoslaw Guzinski"

import os
import pathlib

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import (
    QgsProcessingAlgorithm,
    QgsProcessing,
    QgsProcessingParameterDefinition,
    QgsProperty,
    QgsProcessingParameters,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterEnum,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterRasterDestination,
    QgsProcessingParameterCrs,
    QgsProcessingParameterString,
    QgsProcessingOutputLayerDefinition,
    QgsProcessingUtils,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class buildvrt(GdalAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    RESOLUTION = "RESOLUTION"
    SEPARATE = "SEPARATE"
    PROJ_DIFFERENCE = "PROJ_DIFFERENCE"
    ADD_ALPHA = "ADD_ALPHA"
    ASSIGN_CRS = "ASSIGN_CRS"
    RESAMPLING = "RESAMPLING"
    SRC_NODATA = "SRC_NODATA"
    EXTRA = "EXTRA"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        class ParameterVrtDestination(QgsProcessingParameterRasterDestination):

            def __init__(self, name, description):
                super().__init__(name, description)

            def clone(self):
                copy = ParameterVrtDestination(self.name(), self.description())
                return copy

            def defaultFileExtension(self):
                return "vrt"

            def createFileFilter(self):
                return "{} (*.vrt *.VRT)".format(
                    QCoreApplication.translate("GdalAlgorithm", "VRT files")
                )

            def supportedOutputRasterLayerExtensions(self):
                return ["vrt"]

            def parameterAsOutputLayer(self, definition, value, context):
                return super(
                    QgsProcessingParameterRasterDestination, self
                ).parameterAsOutputLayer(definition, value, context)

            def isSupportedOutputValue(self, value, context):
                output_path = QgsProcessingParameters.parameterAsOutputLayer(
                    self, value, context, testOnly=True
                )
                if pathlib.Path(output_path).suffix.lower() != ".vrt":
                    return False, QCoreApplication.translate(
                        "GdalAlgorithm", "Output filename must use a .vrt extension"
                    )
                return True, ""

        self.RESAMPLING_OPTIONS = (
            (self.tr("Nearest Neighbour"), "nearest"),
            (self.tr("Bilinear (2x2 Kernel)"), "bilinear"),
            (self.tr("Cubic (4x4 Kernel)"), "cubic"),
            (self.tr("Cubic B-Spline (4x4 Kernel)"), "cubicspline"),
            (self.tr("Lanczos (6x6 Kernel)"), "lanczos"),
            (self.tr("Average"), "average"),
            (self.tr("Mode"), "mode"),
        )

        self.RESOLUTION_OPTIONS = (
            (self.tr("Average"), "average"),
            (self.tr("Highest"), "highest"),
            (self.tr("Lowest"), "lowest"),
        )

        self.addParameter(
            QgsProcessingParameterMultipleLayers(
                self.INPUT, self.tr("Input layers"), QgsProcessing.SourceType.TypeRaster
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.RESOLUTION,
                self.tr("Resolution"),
                options=[i[0] for i in self.RESOLUTION_OPTIONS],
                defaultValue=0,
            )
        )

        separate_param = QgsProcessingParameterBoolean(
            self.SEPARATE,
            self.tr("Place each input file into a separate band"),
            defaultValue=True,
        )
        # default to not using separate bands is a friendlier option, but we can't change the parameter's actual
        # defaultValue without breaking API!
        separate_param.setGuiDefaultValueOverride(False)
        self.addParameter(separate_param)

        self.addParameter(
            QgsProcessingParameterBoolean(
                self.PROJ_DIFFERENCE,
                self.tr("Allow projection difference"),
                defaultValue=False,
            )
        )

        add_alpha_param = QgsProcessingParameterBoolean(
            self.ADD_ALPHA,
            self.tr("Add alpha mask band to VRT when source raster has none"),
            defaultValue=False,
        )
        add_alpha_param.setFlags(
            add_alpha_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(add_alpha_param)

        assign_crs = QgsProcessingParameterCrs(
            self.ASSIGN_CRS,
            self.tr("Override projection for the output file"),
            defaultValue=None,
            optional=True,
        )
        assign_crs.setFlags(
            assign_crs.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(assign_crs)

        resampling = QgsProcessingParameterEnum(
            self.RESAMPLING,
            self.tr("Resampling algorithm"),
            options=[i[0] for i in self.RESAMPLING_OPTIONS],
            defaultValue=0,
        )
        resampling.setFlags(
            resampling.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(resampling)

        src_nodata_param = QgsProcessingParameterString(
            self.SRC_NODATA,
            self.tr("Nodata value(s) for input bands (space separated)"),
            defaultValue=None,
            optional=True,
        )
        src_nodata_param.setFlags(
            src_nodata_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(src_nodata_param)

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
            ParameterVrtDestination(
                self.OUTPUT,
                QCoreApplication.translate("ParameterVrtDestination", "Virtual"),
            )
        )

    def name(self):
        return "buildvirtualraster"

    def displayName(self):
        return QCoreApplication.translate("buildvrt", "Build virtual raster")

    def icon(self):
        return QIcon(os.path.join(pluginPath, "images", "gdaltools", "vrt.png"))

    def group(self):
        return QCoreApplication.translate("buildvrt", "Raster miscellaneous")

    def groupId(self):
        return "rastermiscellaneous"

    def commandName(self):
        return "gdalbuildvrt"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = [
            "-overwrite",
            "-resolution",
            self.RESOLUTION_OPTIONS[
                self.parameterAsEnum(parameters, self.RESOLUTION, context)
            ][1],
        ]

        if self.parameterAsBoolean(parameters, buildvrt.SEPARATE, context):
            arguments.append("-separate")
        if self.parameterAsBoolean(parameters, buildvrt.PROJ_DIFFERENCE, context):
            arguments.append("-allow_projection_difference")
        if self.parameterAsBoolean(parameters, buildvrt.ADD_ALPHA, context):
            arguments.append("-addalpha")
        crs = self.parameterAsCrs(parameters, self.ASSIGN_CRS, context)
        if crs.isValid():
            arguments.append("-a_srs")
            arguments.append(GdalUtils.gdal_crs_string(crs))
        arguments.append("-r")
        arguments.append(
            self.RESAMPLING_OPTIONS[
                self.parameterAsEnum(parameters, self.RESAMPLING, context)
            ][1]
        )

        if self.SRC_NODATA in parameters and parameters[self.SRC_NODATA] not in (
            None,
            "",
        ):
            nodata = self.parameterAsString(parameters, self.SRC_NODATA, context)
            arguments.append("-srcnodata")
            arguments.append(nodata)

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        list_file = GdalUtils.writeLayerParameterToTextFile(
            filename="buildvrtInputFiles.txt",
            alg=self,
            parameters=parameters,
            parameter_name=self.INPUT,
            context=context,
            executing=executing,
            quote=False,
        )
        arguments.append("-input_file_list")
        arguments.append(list_file)

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
