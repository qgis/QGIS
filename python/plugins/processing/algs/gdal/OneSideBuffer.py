"""
***************************************************************************
    OneSideBuffer.py
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
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterDistance,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterEnum,
    QgsProcessingParameterField,
    QgsProcessingParameterString,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class OneSideBuffer(GdalAlgorithm):
    INPUT = "INPUT"
    FIELD = "FIELD"
    BUFFER_SIDE = "BUFFER_SIDE"
    GEOMETRY = "GEOMETRY"
    DISTANCE = "DISTANCE"
    DISSOLVE = "DISSOLVE"
    EXPLODE_COLLECTIONS = "EXPLODE_COLLECTIONS"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.bufferSides = [self.tr("Right"), self.tr("Left")]

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
            QgsProcessingParameterDistance(
                self.DISTANCE,
                self.tr("Buffer distance"),
                defaultValue=10,
                parentParameterName=self.INPUT,
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.BUFFER_SIDE,
                self.tr("Buffer side"),
                options=self.bufferSides,
                allowMultiple=False,
                defaultValue=0,
            )
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.FIELD,
                self.tr("Dissolve by attribute"),
                None,
                self.INPUT,
                QgsProcessingParameterField.DataType.Any,
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.DISSOLVE, self.tr("Dissolve all results"), defaultValue=False
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.EXPLODE_COLLECTIONS,
                self.tr(
                    "Produce one feature for each geometry in any kind of geometry collection in the source file"
                ),
                defaultValue=False,
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
                self.tr("One-sided buffer"),
                QgsProcessing.SourceType.TypeVectorPolygon,
            )
        )

    def name(self):
        return "onesidebuffer"

    def displayName(self):
        return self.tr("One side buffer")

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
        geometry = self.parameterAsString(parameters, self.GEOMETRY, context)
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        side = self.parameterAsEnum(parameters, self.BUFFER_SIDE, context)
        fieldName = self.parameterAsString(parameters, self.FIELD, context)
        dissolve = self.parameterAsBoolean(parameters, self.DISSOLVE, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        other_fields_exist = any(True for f in fields if f.name() != geometry)

        other_fields = ",*" if other_fields_exist else ""

        arguments = [
            output_details.connection_string,
            input_details.connection_string,
            "-dialect",
            "sqlite",
            "-sql",
        ]

        if dissolve or fieldName:
            sql = f'SELECT ST_Union(ST_SingleSidedBuffer({geometry}, {distance}, {side})) AS {geometry}{other_fields} FROM "{input_details.layer_name}"'
        else:
            sql = f'SELECT ST_SingleSidedBuffer({geometry}, {distance}, {side}) AS {geometry}{other_fields} FROM "{input_details.layer_name}"'

        if fieldName:
            sql = f'{sql} GROUP BY "{fieldName}"'

        arguments.append(sql)

        if self.parameterAsBoolean(parameters, self.EXPLODE_COLLECTIONS, context):
            arguments.append("-explodecollections")

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if options:
            arguments.append(options)

        if output_details.format:
            arguments.append(f"-f {output_details.format}")

        return ["ogr2ogr", GdalUtils.escapeAndJoin(arguments)]
