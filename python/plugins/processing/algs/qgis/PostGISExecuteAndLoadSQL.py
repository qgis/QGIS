"""
***************************************************************************
    PostGISExecuteAndLoadSQL.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Anita Graser
    Email                : anitagraser at gmx dot at
    ---------------------
    based on PostGISExecuteSQL.py by  Victor Olaya and Carterix Geomatics
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Anita Graser"
__date__ = "May 2018"
__copyright__ = "(C) 2018, Anita Graser"

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterString,
    QgsVectorLayer,
    QgsDataSourceUri,
    QgsProcessing,
    QgsProcessingOutputVectorLayer,
    QgsProcessingContext,
    QgsProcessingParameterProviderConnection,
    QgsProviderRegistry,
    QgsProviderConnectionException,
    QgsProcessingAlgorithm,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PostGISExecuteAndLoadSQL(QgisAlgorithm):
    DATABASE = "DATABASE"
    SQL = "SQL"
    OUTPUT = "OUTPUT"
    ID_FIELD = "ID_FIELD"
    GEOMETRY_FIELD = "GEOMETRY_FIELD"

    def group(self):
        return self.tr("Database")

    def groupId(self):
        return "database"

    def __init__(self):
        super().__init__()

    def flags(self):
        return (
            super().flags()
            | QgsProcessingAlgorithm.Flag.FlagNotAvailableInStandaloneTool
            | QgsProcessingAlgorithm.Flag.FlagRequiresProject
        )

    def initAlgorithm(self, config=None):
        db_param = QgsProcessingParameterProviderConnection(
            self.DATABASE, self.tr("Database (connection name)"), "postgres"
        )
        self.addParameter(db_param)
        self.addParameter(
            QgsProcessingParameterString(self.SQL, self.tr("SQL query"), multiLine=True)
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.ID_FIELD, self.tr("Unique ID field name"), defaultValue="id"
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GEOMETRY_FIELD,
                self.tr("Geometry field name"),
                defaultValue="geom",
                optional=True,
            )
        )
        self.addOutput(
            QgsProcessingOutputVectorLayer(
                self.OUTPUT,
                self.tr("Output layer"),
                QgsProcessing.SourceType.TypeVectorAnyGeometry,
            )
        )

    def name(self):
        return "postgisexecuteandloadsql"

    def displayName(self):
        return self.tr("PostgreSQL execute and load SQL")

    def shortDescription(self):
        return self.tr(
            "Executes a SQL command on a PostgreSQL database and loads the result as a table"
        )

    def tags(self):
        return self.tr("postgis,table,database").split(",")

    def processAlgorithm(self, parameters, context, feedback):
        connection_name = self.parameterAsConnectionName(
            parameters, self.DATABASE, context
        )
        id_field = self.parameterAsString(parameters, self.ID_FIELD, context)
        geom_field = self.parameterAsString(parameters, self.GEOMETRY_FIELD, context)

        # resolve connection details to uri
        try:
            md = QgsProviderRegistry.instance().providerMetadata("postgres")
            conn = md.createConnection(connection_name)
        except QgsProviderConnectionException:
            raise QgsProcessingException(
                self.tr("Could not retrieve connection details for {}").format(
                    connection_name
                )
            )

        uri = QgsDataSourceUri(conn.uri())

        sql = self.parameterAsString(parameters, self.SQL, context)
        sql = sql.replace("\n", " ")
        uri.setDataSource("", "(" + sql.rstrip(";") + ")", geom_field, "", id_field)

        vlayer = QgsVectorLayer(uri.uri(), "layername", "postgres")

        if not vlayer.isValid():
            raise QgsProcessingException(
                self.tr(
                    """This layer is invalid!
                Please check the PostGIS log for error messages."""
                )
            )

        context.temporaryLayerStore().addMapLayer(vlayer)
        context.addLayerToLoadOnCompletion(
            vlayer.id(),
            QgsProcessingContext.LayerDetails(
                "SQL layer", context.project(), self.OUTPUT
            ),
        )

        return {self.OUTPUT: vlayer.id()}
