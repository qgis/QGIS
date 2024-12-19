"""
***************************************************************************
    self.py
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
    QgsProcessingParameterCrs,
    QgsProcessingParameterNumber,
    QgsProcessingParameterEnum,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterExtent,
    QgsProcessingParameterString,
    QgsProcessingParameterRasterDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class warp(GdalAlgorithm):

    INPUT = "INPUT"
    SOURCE_CRS = "SOURCE_CRS"
    TARGET_CRS = "TARGET_CRS"
    NODATA = "NODATA"
    TARGET_RESOLUTION = "TARGET_RESOLUTION"
    OPTIONS = "OPTIONS"
    RESAMPLING = "RESAMPLING"
    DATA_TYPE = "DATA_TYPE"
    TARGET_EXTENT = "TARGET_EXTENT"
    TARGET_EXTENT_CRS = "TARGET_EXTENT_CRS"
    MULTITHREADING = "MULTITHREADING"
    EXTRA = "EXTRA"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = (
            (self.tr("Nearest Neighbour"), "near"),
            (self.tr("Bilinear (2x2 Kernel)"), "bilinear"),
            (self.tr("Cubic (4x4 Kernel)"), "cubic"),
            (self.tr("Cubic B-Spline (4x4 Kernel)"), "cubicspline"),
            (self.tr("Lanczos (6x6 Kernel)"), "lanczos"),
            (self.tr("Average"), "average"),
            (self.tr("Mode"), "mode"),
            (self.tr("Maximum"), "max"),
            (self.tr("Minimum"), "min"),
            (self.tr("Median"), "med"),
            (self.tr("First Quartile (Q1)"), "q1"),
            (self.tr("Third Quartile (Q3)"), "q3"),
        )

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
            QgsProcessingParameterCrs(
                self.SOURCE_CRS, self.tr("Source CRS"), optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterCrs(
                self.TARGET_CRS, self.tr("Target CRS"), optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.RESAMPLING,
                self.tr("Resampling method to use"),
                options=[i[0] for i in self.methods],
                defaultValue=0,
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.NODATA,
                self.tr("Nodata value for output bands"),
                type=QgsProcessingParameterNumber.Type.Double,
                defaultValue=None,
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.TARGET_RESOLUTION,
                self.tr("Output file resolution in target georeferenced units"),
                type=QgsProcessingParameterNumber.Type.Double,
                minValue=0.0,
                defaultValue=None,
                optional=True,
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

        target_extent_param = QgsProcessingParameterExtent(
            self.TARGET_EXTENT,
            self.tr("Georeferenced extents of output file to be created"),
            optional=True,
        )
        target_extent_param.setFlags(
            target_extent_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(target_extent_param)

        target_extent_crs_param = QgsProcessingParameterCrs(
            self.TARGET_EXTENT_CRS,
            self.tr("CRS of the target raster extent"),
            optional=True,
        )
        target_extent_crs_param.setFlags(
            target_extent_crs_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(target_extent_crs_param)

        multithreading_param = QgsProcessingParameterBoolean(
            self.MULTITHREADING,
            self.tr("Use multithreaded warping implementation"),
            defaultValue=False,
        )
        multithreading_param.setFlags(
            multithreading_param.flags()
            | QgsProcessingParameterDefinition.Flag.FlagAdvanced
        )
        self.addParameter(multithreading_param)

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
            QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr("Reprojected"))
        )

    def name(self):
        return "warpreproject"

    def displayName(self):
        return self.tr("Warp (reproject)")

    def group(self):
        return self.tr("Raster projections")

    def groupId(self):
        return "rasterprojections"

    def icon(self):
        return QIcon(os.path.join(pluginPath, "images", "gdaltools", "warp.png"))

    def commandName(self):
        return "gdalwarp"

    def tags(self):
        tags = self.tr("transform,reproject,crs,srs,resample").split(",")
        tags.extend(super().tags())
        return tags

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )
        input_details = GdalUtils.gdal_connection_details_from_layer(inLayer)

        sourceCrs = self.parameterAsCrs(parameters, self.SOURCE_CRS, context)
        targetCrs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        resolution = self.parameterAsDouble(parameters, self.TARGET_RESOLUTION, context)

        arguments = ["-overwrite"]
        if sourceCrs.isValid():
            arguments.append("-s_srs")
            arguments.append(GdalUtils.gdal_crs_string(sourceCrs))

        if targetCrs.isValid():
            arguments.append("-t_srs")
            arguments.append(GdalUtils.gdal_crs_string(targetCrs))

        if nodata is not None:
            arguments.append("-dstnodata")
            arguments.append(str(nodata))

        if resolution:
            arguments.append("-tr")
            arguments.append(str(resolution))
            arguments.append(str(resolution))

        arguments.append("-r")
        arguments.append(
            self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1]
        )

        extent = self.parameterAsExtent(parameters, self.TARGET_EXTENT, context)
        if not extent.isNull():
            arguments.append("-te")
            arguments.append(extent.xMinimum())
            arguments.append(extent.yMinimum())
            arguments.append(extent.xMaximum())
            arguments.append(extent.yMaximum())

            extentCrs = self.parameterAsCrs(parameters, self.TARGET_EXTENT_CRS, context)
            if extentCrs.isValid():
                arguments.append("-te_srs")
                arguments.append(GdalUtils.gdal_crs_string(extentCrs))

        if self.parameterAsBoolean(parameters, self.MULTITHREADING, context):
            arguments.append("-multi")

        data_type = self.parameterAsEnum(parameters, self.DATA_TYPE, context)
        if data_type:
            if self.TYPES[data_type] == "Int8" and GdalUtils.version() < 3070000:
                raise QgsProcessingException(
                    self.tr("Int8 data type requires GDAL version 3.7 or later")
                )

            arguments.append("-ot " + self.TYPES[data_type])

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)

        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1])
        if not output_format:
            raise QgsProcessingException(self.tr("Output format is invalid"))

        arguments.append("-of")
        arguments.append(output_format)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ""):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        arguments.append(input_details.connection_string)
        arguments.append(out)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
