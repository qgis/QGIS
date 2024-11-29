"""
***************************************************************************
    Dissolve.py
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
    QgsProcessingParameterDefinition,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterString,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class Dissolve(GdalAlgorithm):
    INPUT = "INPUT"
    FIELD = "FIELD"
    GEOMETRY = "GEOMETRY"
    EXPLODE_COLLECTIONS = "EXPLODE_COLLECTIONS"
    KEEP_ATTRIBUTES = "KEEP_ATTRIBUTES"
    COUNT_FEATURES = "COUNT_FEATURES"
    COMPUTE_AREA = "COMPUTE_AREA"
    COMPUTE_STATISTICS = "COMPUTE_STATISTICS"
    STATISTICS_ATTRIBUTE = "STATISTICS_ATTRIBUTE"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.FIELD,
                self.tr("Dissolve field"),
                None,
                self.INPUT,
                QgsProcessingParameterField.DataType.Any,
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GEOMETRY, self.tr("Geometry column name"), defaultValue="geometry"
            )
        )
        params = [
            QgsProcessingParameterBoolean(
                self.EXPLODE_COLLECTIONS,
                self.tr(
                    "Produce one feature for each geometry in any kind of geometry collection in the source file"
                ),
                defaultValue=False,
            ),
            QgsProcessingParameterBoolean(
                self.KEEP_ATTRIBUTES,
                self.tr("Keep input attributes"),
                defaultValue=False,
            ),
            QgsProcessingParameterBoolean(
                self.COUNT_FEATURES,
                self.tr("Count dissolved features"),
                defaultValue=False,
            ),
            QgsProcessingParameterBoolean(
                self.COMPUTE_AREA,
                self.tr("Compute area and perimeter of dissolved features"),
                defaultValue=False,
            ),
            QgsProcessingParameterBoolean(
                self.COMPUTE_STATISTICS,
                self.tr("Compute min/max/sum/mean for attribute"),
                defaultValue=False,
            ),
            QgsProcessingParameterField(
                self.STATISTICS_ATTRIBUTE,
                self.tr("Numeric attribute to calculate statistics on"),
                None,
                self.INPUT,
                QgsProcessingParameterField.DataType.Numeric,
                optional=True,
            ),
            QgsProcessingParameterString(
                self.OPTIONS,
                self.tr("Additional creation options"),
                defaultValue="",
                optional=True,
            ),
        ]
        for param in params:
            param.setFlags(
                param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
            )
            self.addParameter(param)

        self.addParameter(
            QgsProcessingParameterVectorDestination(self.OUTPUT, self.tr("Dissolved"))
        )

    def name(self):
        return "dissolve"

    def displayName(self):
        return self.tr("Dissolve")

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
        fieldName = self.parameterAsString(parameters, self.FIELD, context)

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        other_fields_exist = any(True for f in fields if f.name() != geometry)

        other_fields = ",*" if other_fields_exist else ""

        arguments = [
            output_details.connection_string,
            input_details.connection_string,
            "-nlt PROMOTE_TO_MULTI",
            "-dialect",
            "sqlite",
            "-sql",
        ]

        tokens = []
        if self.parameterAsBoolean(parameters, self.COUNT_FEATURES, context):
            tokens.append(f"COUNT({geometry}) AS count")

        if self.parameterAsBoolean(parameters, self.COMPUTE_AREA, context):
            tokens.append(
                "SUM(ST_Area({0})) AS area, ST_Perimeter(ST_Union({0})) AS perimeter".format(
                    geometry
                )
            )

        statsField = self.parameterAsString(
            parameters, self.STATISTICS_ATTRIBUTE, context
        )
        if statsField and self.parameterAsBoolean(
            parameters, self.COMPUTE_STATISTICS, context
        ):
            tokens.append(
                'SUM("{0}") AS sum, MIN("{0}") AS min, MAX("{0}") AS max, AVG("{0}") AS avg'.format(
                    statsField
                )
            )

        params = ",".join(tokens)
        if params:
            params = ", " + params

        group_by = ""
        if fieldName:
            group_by = f' GROUP BY "{fieldName}"'

        if self.parameterAsBoolean(parameters, self.KEEP_ATTRIBUTES, context):
            sql = f'SELECT ST_Union({geometry}) AS {geometry}{other_fields}{params} FROM "{input_details.layer_name}"{group_by}'
        else:
            sql = 'SELECT ST_Union({}) AS {}{}{} FROM "{}"{}'.format(
                geometry,
                geometry,
                f', "{fieldName}"' if fieldName else "",
                params,
                input_details.layer_name,
                group_by,
            )

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

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
