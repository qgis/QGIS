"""
***************************************************************************
    ClipVectorByExtent.py
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

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterExtent,
    QgsProcessingParameterString,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ClipVectorByExtent(GdalAlgorithm):
    INPUT = "INPUT"
    EXTENT = "EXTENT"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterExtent(self.EXTENT, self.tr("Clipping extent"))
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
        self.addParameter(options_param)

        self.addParameter(
            QgsProcessingParameterVectorDestination(
                self.OUTPUT, self.tr("Clipped (extent)")
            )
        )

    def name(self):
        return "clipvectorbyextent"

    def displayName(self):
        return self.tr("Clip vector by extent")

    def group(self):
        return self.tr("Vector geoprocessing")

    def groupId(self):
        return "vectorgeoprocessing"

    def commandName(self):
        return "ogr2ogr"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        extent = self.parameterAsExtent(
            parameters, self.EXTENT, context, source.sourceCrs()
        )
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        arguments = [
            "-spat",
            str(extent.xMinimum()),
            str(extent.yMaximum()),
            str(extent.xMaximum()),
            str(extent.yMinimum()),
            "-clipsrc spat_extent",
            output_details.connection_string,
            input_details.connection_string,
            input_details.layer_name,
        ]

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if options:
            arguments.append(options)

        if output_details.format:
            arguments.append(f"-f {output_details.format}")

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
