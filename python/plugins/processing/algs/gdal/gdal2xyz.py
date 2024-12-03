"""
***************************************************************************
    gdal2xyz.py
    ---------------------
    Date                 : September 2013
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
__date__ = "September 2013"
__copyright__ = "(C) 2013, Alexander Bruy"

from qgis.core import (
    QgsProcessingAlgorithm,
    QgsProcessingException,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterBand,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterFileDestination,
    QgsProcessingParameterNumber,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows


class gdal2xyz(GdalAlgorithm):
    INPUT = "INPUT"
    BAND = "BAND"
    SRCNODATA = "NODATA_INPUT"
    DSTNODATA = "NODATA_OUTPUT"
    SKIPNODATA = "SKIP_NODATA"
    CSV = "CSV"
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
                self.SRCNODATA,
                self.tr("Input pixel value to treat as NoData"),
                optional=True,
            )
        )  # GDAL > 3.6.3
        self.addParameter(
            QgsProcessingParameterNumber(
                self.DSTNODATA,
                self.tr("Assign specified NoData value to output"),
                optional=True,
            )
        )  # GDAL > 3.6.3
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.SKIPNODATA,
                self.tr("Do not output NoData values"),
                defaultValue=False,
            )
        )  # GDAL > 3.3
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.CSV, self.tr("Output comma-separated values"), defaultValue=False
            )
        )
        self.addParameter(
            QgsProcessingParameterFileDestination(
                self.OUTPUT, self.tr("XYZ ASCII file"), self.tr("CSV files (*.csv)")
            )
        )

    def name(self):
        return "gdal2xyz"

    def displayName(self):
        return self.tr("gdal2xyz")

    def group(self):
        return self.tr("Raster conversion")

    def groupId(self):
        return "rasterconversion"

    def commandName(self):
        return "gdal2xyz"

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.Flag.FlagDisplayNameIsLiteral

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = ["-band", str(self.parameterAsInt(parameters, self.BAND, context))]

        if self.SRCNODATA in parameters and parameters[self.SRCNODATA] is not None:
            if (
                GdalUtils.version() > 3060300
            ):  # src/dstnodata broken <= 3.6.3 https://github.com/OSGeo/gdal/issues/7410
                srcnodata = self.parameterAsDouble(parameters, self.SRCNODATA, context)
                arguments.append("-srcnodata")
                arguments.append(srcnodata)
            else:
                raise QgsProcessingException(
                    self.tr(
                        "The source nodata option (-srcnodata) is only available on GDAL 3.6.4 or later"
                    )
                )

        if self.DSTNODATA in parameters and parameters[self.DSTNODATA] is not None:
            if (
                GdalUtils.version() > 3060300
            ):  # src/dstnodata broken <= 3.6.3 https://github.com/OSGeo/gdal/issues/7410
                dstnodata = self.parameterAsDouble(parameters, self.DSTNODATA, context)
                arguments.append("-dstnodata")
                arguments.append(dstnodata)
            else:
                raise QgsProcessingException(
                    self.tr(
                        "The destination nodata option (-dstnodata) is only available on GDAL 3.6.4 or later"
                    )
                )

        if self.SKIPNODATA in parameters:
            if GdalUtils.version() >= 3030000:  # skipnodata added at GDAL 3.3
                if self.parameterAsBoolean(parameters, self.SKIPNODATA, context):
                    arguments.append("-skipnodata")
            else:
                raise QgsProcessingException(
                    self.tr(
                        "The skip nodata option (-skipnodata) is only available on GDAL 3.3 or later"
                    )
                )

        if self.parameterAsBoolean(parameters, self.CSV, context):
            arguments.append("-csv")

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(
                self.invalidRasterError(parameters, self.INPUT)
            )

        input_details = GdalUtils.gdal_connection_details_from_layer(raster)

        arguments.append(input_details.connection_string)
        arguments.append(self.parameterAsFileOutput(parameters, self.OUTPUT, context))

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        return [
            self.commandName() + (".bat" if isWindows() else ".py"),
            GdalUtils.escapeAndJoin(arguments),
        ]
