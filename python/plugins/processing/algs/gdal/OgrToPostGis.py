"""
***************************************************************************
    OgrToPostGis.py
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
    QgsProcessing,
    QgsProcessingException,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterString,
    QgsProcessingParameterEnum,
    QgsProcessingParameterCrs,
    QgsProcessingParameterField,
    QgsProcessingParameterExtent,
    QgsProcessingParameterBoolean,
)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows


class OgrToPostGis(GdalAlgorithm):
    INPUT = "INPUT"
    SHAPE_ENCODING = "SHAPE_ENCODING"
    GTYPE = "GTYPE"
    GEOMTYPE = [
        "",
        "NONE",
        "GEOMETRY",
        "POINT",
        "LINESTRING",
        "POLYGON",
        "GEOMETRYCOLLECTION",
        "MULTIPOINT",
        "MULTIPOLYGON",
        "MULTILINESTRING",
        "CIRCULARSTRING",
        "COMPOUNDCURVE",
        "CURVEPOLYGON",
        "MULTICURVE",
        "MULTISURFACE",
        "CONVERT_TO_LINEAR",
        "CONVERT_TO_CURVE",
    ]
    S_SRS = "S_SRS"
    T_SRS = "T_SRS"
    A_SRS = "A_SRS"
    HOST = "HOST"
    PORT = "PORT"
    USER = "USER"
    DBNAME = "DBNAME"
    PASSWORD = "PASSWORD"
    SCHEMA = "SCHEMA"
    TABLE = "TABLE"
    PK = "PK"
    PRIMARY_KEY = "PRIMARY_KEY"
    GEOCOLUMN = "GEOCOLUMN"
    DIM = "DIM"
    DIMLIST = ["2", "3", "4"]
    SIMPLIFY = "SIMPLIFY"
    SEGMENTIZE = "SEGMENTIZE"
    SPAT = "SPAT"
    CLIP = "CLIP"
    FIELDS = "FIELDS"
    WHERE = "WHERE"
    GT = "GT"
    OVERWRITE = "OVERWRITE"
    APPEND = "APPEND"
    ADDFIELDS = "ADDFIELDS"
    LAUNDER = "LAUNDER"
    INDEX = "INDEX"
    SKIPFAILURES = "SKIPFAILURES"
    PRECISION = "PRECISION"
    MAKEVALID = "MAKEVALID"
    PROMOTETOMULTI = "PROMOTETOMULTI"
    OPTIONS = "OPTIONS"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Input layer"),
                types=[QgsProcessing.SourceType.TypeVector],
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.SHAPE_ENCODING, self.tr("Shape encoding"), "", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.GTYPE,
                self.tr("Output geometry type"),
                options=self.GEOMTYPE,
                defaultValue=0,
            )
        )
        self.addParameter(
            QgsProcessingParameterCrs(
                self.A_SRS,
                self.tr("Assign an output CRS"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterCrs(
                self.T_SRS,
                self.tr("Reproject to this CRS on output "),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterCrs(
                self.S_SRS,
                self.tr("Override source CRS"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.HOST, self.tr("Host"), defaultValue="localhost", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.PORT, self.tr("Port"), defaultValue="5432", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.USER, self.tr("Username"), defaultValue="", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.DBNAME, self.tr("Database name"), defaultValue="", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.PASSWORD, self.tr("Password"), defaultValue="", optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.SCHEMA,
                self.tr("Schema name"),
                defaultValue="public",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.TABLE,
                self.tr("Table name, leave blank to use input name"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.PK,
                self.tr("Primary key (new field)"),
                defaultValue="id",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.PRIMARY_KEY,
                self.tr(
                    "Primary key (existing field, used if the above option is left empty)"
                ),
                parentLayerParameterName=self.INPUT,
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GEOCOLUMN,
                self.tr("Geometry column name"),
                defaultValue="geom",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterEnum(
                self.DIM,
                self.tr("Vector dimensions"),
                options=self.DIMLIST,
                defaultValue=0,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.SIMPLIFY,
                self.tr("Distance tolerance for simplification"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.SEGMENTIZE,
                self.tr("Maximum distance between 2 nodes (densification)"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterExtent(
                self.SPAT,
                self.tr("Select features by extent (defined in input layer CRS)"),
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.CLIP,
                self.tr("Clip the input layer using the above (rectangle) extent"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterField(
                self.FIELDS,
                self.tr("Fields to include (leave empty to use all fields)"),
                parentLayerParameterName=self.INPUT,
                allowMultiple=True,
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.WHERE,
                self.tr(
                    "Select features using a SQL \"WHERE\" statement (Ex: column='value')"
                ),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GT,
                self.tr("Group N features per transaction (Default: 20000)"),
                defaultValue="",
                optional=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.OVERWRITE, self.tr("Overwrite existing table"), defaultValue=True
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.APPEND, self.tr("Append to existing table"), defaultValue=False
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.ADDFIELDS,
                self.tr("Append and add new fields to existing table"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.LAUNDER,
                self.tr("Do not launder columns/table names"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.INDEX, self.tr("Do not create spatial index"), defaultValue=False
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.SKIPFAILURES,
                self.tr("Continue after a failure, skipping the failed feature"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.MAKEVALID,
                self.tr("Validate geometries based on Simple Features specification"),
                defaultValue=False,
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.PROMOTETOMULTI, self.tr("Promote to Multipart"), defaultValue=True
            )
        )
        self.addParameter(
            QgsProcessingParameterBoolean(
                self.PRECISION,
                self.tr("Keep width and precision of input attributes"),
                defaultValue=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.OPTIONS,
                self.tr("Additional creation options"),
                defaultValue="",
                optional=True,
            )
        )

    def name(self):
        return "importvectorintopostgisdatabasenewconnection"

    def displayName(self):
        return self.tr("Export to PostgreSQL (new connection)")

    def shortDescription(self):
        return self.tr("Exports a vector layer to a new PostgreSQL database connection")

    def tags(self):
        t = self.tr("import,into,postgis,database,vector").split(",")
        t.extend(super().tags())
        return t

    def group(self):
        return self.tr("Vector miscellaneous")

    def groupId(self):
        return "vectormiscellaneous"

    def getConnectionString(self, parameters, context):
        host = self.parameterAsString(parameters, self.HOST, context)
        port = self.parameterAsString(parameters, self.PORT, context)
        user = self.parameterAsString(parameters, self.USER, context)
        dbname = self.parameterAsString(parameters, self.DBNAME, context)
        password = self.parameterAsString(parameters, self.PASSWORD, context)
        schema = self.parameterAsString(parameters, self.SCHEMA, context)
        arguments = []
        if host:
            arguments.append("host=" + host)
        if port:
            arguments.append("port=" + str(port))
        if dbname:
            arguments.append("dbname=" + dbname)
        if password:
            arguments.append("password=" + password)
        if schema:
            arguments.append("active_schema=" + schema)
        if user:
            arguments.append("user=" + user)
        return GdalUtils.escapeAndJoin(arguments)

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_details = self.getOgrCompatibleSource(
            self.INPUT, parameters, context, feedback, executing
        )
        if not input_details.layer_name:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        shapeEncoding = self.parameterAsString(parameters, self.SHAPE_ENCODING, context)
        ssrs = self.parameterAsCrs(parameters, self.S_SRS, context)
        tsrs = self.parameterAsCrs(parameters, self.T_SRS, context)
        asrs = self.parameterAsCrs(parameters, self.A_SRS, context)
        table = self.parameterAsString(parameters, self.TABLE, context)
        schema = self.parameterAsString(parameters, self.SCHEMA, context)
        pk = self.parameterAsString(parameters, self.PK, context)
        pkstring = "-lco FID=" + pk
        primary_key = self.parameterAsString(parameters, self.PRIMARY_KEY, context)
        geocolumn = self.parameterAsString(parameters, self.GEOCOLUMN, context)
        geocolumnstring = "-lco GEOMETRY_NAME=" + geocolumn
        dim = self.DIMLIST[self.parameterAsEnum(parameters, self.DIM, context)]
        dimstring = "-lco DIM=" + dim
        simplify = self.parameterAsString(parameters, self.SIMPLIFY, context)
        segmentize = self.parameterAsString(parameters, self.SEGMENTIZE, context)
        spat = self.parameterAsExtent(parameters, self.SPAT, context)
        clip = self.parameterAsBoolean(parameters, self.CLIP, context)
        include_fields = self.parameterAsFields(parameters, self.FIELDS, context)
        fields_string = '-select "' + ",".join(include_fields) + '"'
        where = self.parameterAsString(parameters, self.WHERE, context)
        wherestring = '-where "' + where + '"'
        gt = self.parameterAsString(parameters, self.GT, context)
        overwrite = self.parameterAsBoolean(parameters, self.OVERWRITE, context)
        append = self.parameterAsBoolean(parameters, self.APPEND, context)
        addfields = self.parameterAsBoolean(parameters, self.ADDFIELDS, context)
        launder = self.parameterAsBoolean(parameters, self.LAUNDER, context)
        launderstring = "-lco LAUNDER=NO"
        index = self.parameterAsBoolean(parameters, self.INDEX, context)
        indexstring = "-lco SPATIAL_INDEX=OFF"
        skipfailures = self.parameterAsBoolean(parameters, self.SKIPFAILURES, context)
        make_valid = self.parameterAsBoolean(parameters, self.MAKEVALID, context)
        promotetomulti = self.parameterAsBoolean(
            parameters, self.PROMOTETOMULTI, context
        )
        precision = self.parameterAsBoolean(parameters, self.PRECISION, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)

        arguments = ["-progress", "--config PG_USE_COPY YES"]
        if len(shapeEncoding) > 0:
            arguments.append("--config")
            arguments.append("SHAPE_ENCODING")
            arguments.append(shapeEncoding)
        arguments.append("-f")
        arguments.append("PostgreSQL")
        arguments.append("PG:" + self.getConnectionString(parameters, context))
        arguments.append(dimstring)
        arguments.append(input_details.connection_string)
        arguments.append(input_details.layer_name)
        if index:
            arguments.append(indexstring)
        if launder:
            arguments.append(launderstring)
        if append and overwrite:
            raise QgsProcessingException(
                self.tr(
                    'Only one of "Overwrite existing table" or "Append to existing table" can be enabled at a time.'
                )
            )
        elif append:
            arguments.append("-append")
        if include_fields:
            arguments.append(fields_string)
        if addfields:
            arguments.append("-addfields")
        if overwrite:
            arguments.append("-overwrite")
        if (
            len(self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)])
            > 0
        ):
            arguments.append("-nlt")
            arguments.append(
                self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)]
            )
        if len(geocolumn) > 0:
            arguments.append(geocolumnstring)
        if pk:
            arguments.append(pkstring)
        elif primary_key:
            arguments.append("-lco FID=" + primary_key)
        if len(table) == 0:
            table = input_details.layer_name.lower()
        if schema:
            table = f"{schema}.{table}"
        arguments.append("-nln")
        arguments.append(table)
        if ssrs.isValid():
            arguments.append("-s_srs")
            arguments.append(GdalUtils.gdal_crs_string(ssrs))
        if tsrs.isValid():
            arguments.append("-t_srs")
            arguments.append(GdalUtils.gdal_crs_string(tsrs))
        if asrs.isValid():
            arguments.append("-a_srs")
            arguments.append(GdalUtils.gdal_crs_string(asrs))
        if not spat.isNull():
            arguments.append("-spat")
            arguments.append(spat.xMinimum())
            arguments.append(spat.yMinimum())
            arguments.append(spat.xMaximum())
            arguments.append(spat.yMaximum())
            if clip:
                arguments.append("-clipsrc spat_extent")
        if skipfailures:
            arguments.append("-skipfailures")
        if where:
            arguments.append(wherestring)
        if len(simplify) > 0:
            arguments.append("-simplify")
            arguments.append(simplify)
        if len(segmentize) > 0:
            arguments.append("-segmentize")
            arguments.append(segmentize)
        if len(gt) > 0:
            arguments.append("-gt")
            arguments.append(gt)
        if make_valid:
            arguments.append("-makevalid")
        if (
            promotetomulti
            and self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)]
        ):
            if (
                self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)]
                == "CONVERT_TO_LINEAR"
            ):
                arguments.append("-nlt PROMOTE_TO_MULTI")
            else:
                raise QgsProcessingException(
                    self.tr(
                        'Only one of "Promote to Multipart" or "Output geometry type" (excluding Convert to Linear) can be enabled.'
                    )
                )
        elif (
            promotetomulti
            and not self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)]
        ):
            arguments.append("-nlt PROMOTE_TO_MULTI")
        if precision is False:
            arguments.append("-lco PRECISION=NO")

        if input_details.open_options:
            arguments.extend(input_details.open_options_as_arguments())

        if input_details.credential_options:
            arguments.extend(input_details.credential_options_as_arguments())

        if len(options) > 0:
            arguments.append(options)

        if isWindows():
            return ["cmd.exe", "/C ", "ogr2ogr.exe", GdalUtils.escapeAndJoin(arguments)]
        else:
            return ["ogr2ogr", GdalUtils.escapeAndJoin(arguments)]

    def commandName(self):
        return "ogr2ogr"
