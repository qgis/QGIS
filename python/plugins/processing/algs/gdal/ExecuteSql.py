"""
***************************************************************************
    ExecuteSql.py
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
    QgsProcessingParameterEnum,
    QgsProcessingParameterString,
    QgsProcessingParameterVectorDestination,
)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class ExecuteSql(GdalAlgorithm):
    INPUT = "INPUT"
    SQL = "SQL"
    DIALECT = "DIALECT"
    OPTIONS = "OPTIONS"
    OUTPUT = "OUTPUT"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.dialects = (
            (self.tr("None"), ""),
            (self.tr("OGR SQL"), "ogrsql"),
            (self.tr("SQLite"), "sqlite"),
        )

        self.addParameter(
            QgsProcessingParameterFeatureSource(self.INPUT, self.tr("Input layer"))
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.SQL, self.tr("SQL expression"), defaultValue=""
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.DIALECT,
                self.tr("SQL dialect"),
                options=[i[0] for i in self.dialects],
                allowMultiple=False,
                defaultValue=0,
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
            QgsProcessingParameterVectorDestination(self.OUTPUT, self.tr("SQL result"))
        )

    def name(self):
        return "executesql"

    def displayName(self):
        return self.tr("Execute SQL")

    def group(self):
        return self.tr("Vector miscellaneous")

    def groupId(self):
        return "vectormiscellaneous"

    def commandName(self):
        return "ogr2ogr"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        sql = self.parameterAsString(parameters, self.SQL, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)

        output_details = GdalUtils.gdal_connection_details_from_uri(outFile, context)

        if not sql:
            raise QgsProcessingException(
                self.tr("Empty SQL. Please enter valid SQL expression and try again.")
            )

        arguments = [
            output_details.connection_string,
            input_details.connection_string,
            "-sql",
            sql,
        ]
        dialect = self.dialects[
            self.parameterAsEnum(parameters, self.DIALECT, context)
        ][1]
        if dialect:
            arguments.append("-dialect")
            arguments.append(dialect)

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if options:
            arguments.append(options)

        if output_details.format:
            arguments.append(f"-f {output_details.format}")

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
