"""
***************************************************************************
    PointsAlongLines.py
    ---------------------
    Date                 : Janaury 2015
    Copyright            : (C) 2015 by Giovanni Manghi
    Email                : giovanni dot manghi at naturalgis dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Giovanni Manghi"
__date__ = "January 2015"
__copyright__ = "(C) 2015, Giovanni Manghi"

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterString,
    QgsProcessingParameterNumber,
    QgsProcessingParameterVectorDestination,
    QgsProcessingParameterDefinition,
    QgsProcessing,
)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class PointsAlongLines(GdalAlgorithm):
    INPUT = "INPUT"
    GEOMETRY = "GEOMETRY"
    DISTANCE = "DISTANCE"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Input layer"),
                [QgsProcessing.SourceType.TypeVectorLine],
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GEOMETRY, self.tr("Geometry column name"), defaultValue="geometry"
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.DISTANCE,
                self.tr(
                    "Distance from line start represented as fraction of line length"
                ),
                type=QgsProcessingParameterNumber.Type.Double,
                minValue=0,
                maxValue=1,
                defaultValue=0.5,
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
        self.addParameter(options_param)

        self.addParameter(
            QgsProcessingParameterVectorDestination(
                self.OUTPUT,
                self.tr("Points along lines"),
                QgsProcessing.SourceType.TypeVectorPoint,
            )
        )

    def name(self):
        return "pointsalonglines"

    def displayName(self):
        return self.tr("Points along lines")

    def group(self):
        return self.tr("Vector geoprocessing")

    def groupId(self):
        return "vectorgeoprocessing"

    def commandName(self):
        return "ogr2ogr"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        fields = source.fields()
        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        geometry = self.parameterAsString(parameters, self.GEOMETRY, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        options = self.parameterAsString(parameters, self.OPTIONS, context)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        other_fields_exist = any(f for f in fields if f.name() != geometry)

        other_fields = ",*" if other_fields_exist else ""

        arguments = [
            output_details.connection_string,
            input_details.connection_string,
            "-dialect",
            "sqlite",
            "-sql",
            f'SELECT ST_Line_Interpolate_Point({geometry}, {distance}) AS {geometry}{other_fields} FROM "{input_details.layer_name}"',
        ]

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if options:
            arguments.append(options)

        if output_details.format:
            arguments.append(f"-f {output_details.format}")

        return ["ogr2ogr", GdalUtils.escapeAndJoin(arguments)]
