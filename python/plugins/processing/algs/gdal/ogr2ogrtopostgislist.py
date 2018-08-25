# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrtopostgislist.py
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

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterField,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterBoolean)

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.postgis import uri_from_name, GeoDB
from processing.tools.system import isWindows


class Ogr2OgrToPostGisList(GdalAlgorithm):
    DATABASE = 'DATABASE'
    INPUT = 'INPUT'
    SHAPE_ENCODING = 'SHAPE_ENCODING'
    GTYPE = 'GTYPE'
    GEOMTYPE = ['', 'NONE', 'GEOMETRY', 'POINT', 'LINESTRING', 'POLYGON', 'GEOMETRYCOLLECTION', 'MULTIPOINT',
                'MULTIPOLYGON', 'MULTILINESTRING']
    S_SRS = 'S_SRS'
    T_SRS = 'T_SRS'
    A_SRS = 'A_SRS'
    HOST = 'HOST'
    PORT = 'PORT'
    USER = 'USER'
    DBNAME = 'DBNAME'
    PASSWORD = 'PASSWORD'
    SCHEMA = 'SCHEMA'
    TABLE = 'TABLE'
    PK = 'PK'
    PRIMARY_KEY = 'PRIMARY_KEY'
    GEOCOLUMN = 'GEOCOLUMN'
    DIM = 'DIM'
    DIMLIST = ['2', '3']
    SIMPLIFY = 'SIMPLIFY'
    SEGMENTIZE = 'SEGMENTIZE'
    SPAT = 'SPAT'
    CLIP = 'CLIP'
    WHERE = 'WHERE'
    GT = 'GT'
    OVERWRITE = 'OVERWRITE'
    APPEND = 'APPEND'
    ADDFIELDS = 'ADDFIELDS'
    LAUNDER = 'LAUNDER'
    INDEX = 'INDEX'
    SKIPFAILURES = 'SKIPFAILURES'
    PRECISION = 'PRECISION'
    PROMOTETOMULTI = 'PROMOTETOMULTI'
    OPTIONS = 'OPTIONS'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        db_param = QgsProcessingParameterString(
            self.DATABASE,
            self.tr('Database (connection name)'))
        db_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.ConnectionWidgetWrapper'}})
        self.addParameter(db_param)
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              types=[QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterString(self.SHAPE_ENCODING,
                                                       self.tr('Shape encoding'), "", optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.GTYPE,
                                                     self.tr('Output geometry type'), options=self.GEOMTYPE,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterCrs(self.A_SRS,
                                                    self.tr('Assign an output CRS'), defaultValue='', optional=False))
        self.addParameter(QgsProcessingParameterCrs(self.T_SRS,
                                                    self.tr('Reproject to this CRS on output '), defaultValue='',
                                                    optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.S_SRS,
                                                    self.tr('Override source CRS'), defaultValue='', optional=True))

        schema_param = QgsProcessingParameterString(
            self.SCHEMA,
            self.tr('Schema (schema name)'), 'public', False, True)
        schema_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.SchemaWidgetWrapper',
                'connection_param': self.DATABASE}})
        self.addParameter(schema_param)

        table_param = QgsProcessingParameterString(
            self.TABLE,
            self.tr('Table to import to (leave blank to use layer name)'), '', False, True)
        table_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.gui.wrappers_postgis.TableWidgetWrapper',
                'schema_param': self.SCHEMA}})
        self.addParameter(table_param)

        self.addParameter(QgsProcessingParameterString(self.PK,
                                                       self.tr('Primary key (new field)'), defaultValue='id',
                                                       optional=True))
        self.addParameter(QgsProcessingParameterField(self.PRIMARY_KEY,
                                                      self.tr(
                                                          'Primary key (existing field, used if the above option is left empty)'),
                                                      parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterString(self.GEOCOLUMN,
                                                       self.tr('Geometry column name'), defaultValue='geom',
                                                       optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.DIM,
                                                     self.tr('Vector dimensions'), options=self.DIMLIST,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterString(self.SIMPLIFY,
                                                       self.tr('Distance tolerance for simplification'),
                                                       defaultValue='', optional=True))
        self.addParameter(QgsProcessingParameterString(self.SEGMENTIZE,
                                                       self.tr('Maximum distance between 2 nodes (densification)'),
                                                       defaultValue='', optional=True))
        self.addParameter(QgsProcessingParameterExtent(self.SPAT,
                                                       self.tr(
                                                           'Select features by extent (defined in input layer CRS)'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.CLIP,
                                                        self.tr(
                                                            'Clip the input layer using the above (rectangle) extent'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterString(self.WHERE,
                                                       self.tr(
                                                           'Select features using a SQL "WHERE" statement (Ex: column=\'value\')'),
                                                       defaultValue='', optional=True))
        self.addParameter(QgsProcessingParameterString(self.GT,
                                                       self.tr('Group N features per transaction (Default: 20000)'),
                                                       defaultValue='', optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.OVERWRITE,
                                                        self.tr('Overwrite existing table'), defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.APPEND,
                                                        self.tr('Append to existing table'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.ADDFIELDS,
                                                        self.tr('Append and add new fields to existing table'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.LAUNDER,
                                                        self.tr('Do not launder columns/table names'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.INDEX,
                                                        self.tr('Do not create spatial index'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.SKIPFAILURES,
                                                        self.tr(
                                                            'Continue after a failure, skipping the failed feature'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.PROMOTETOMULTI,
                                                        self.tr('Promote to Multipart'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.PRECISION,
                                                        self.tr('Keep width and precision of input attributes'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterString(self.OPTIONS,
                                                       self.tr('Additional creation options'), defaultValue='',
                                                       optional=True))

    def name(self):
        return 'importvectorintopostgisdatabaseavailableconnections'

    def displayName(self):
        return self.tr('Export to PostgreSQL (available connections)')

    def shortDescription(self):
        return self.tr('Exports a vector layer to an existing PostgreSQL database connection')

    def tags(self):
        t = self.tr('import,into,postgis,database,vector').split(',')
        t.extend(super().tags())
        return t

    def group(self):
        return self.tr('Vector miscellaneous')

    def groupId(self):
        return 'vectormiscellaneous'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        connection = self.parameterAsString(parameters, self.DATABASE, context)
        uri = uri_from_name(connection)
        if executing:
            # to get credentials input when needed
            uri = GeoDB(uri=uri).uri

        ogrLayer, layername = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback, executing)
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
        clip = self.parameterAsBool(parameters, self.CLIP, context)
        where = self.parameterAsString(parameters, self.WHERE, context)
        wherestring = '-where "' + where + '"'
        gt = self.parameterAsString(parameters, self.GT, context)
        overwrite = self.parameterAsBool(parameters, self.OVERWRITE, context)
        append = self.parameterAsBool(parameters, self.APPEND, context)
        addfields = self.parameterAsBool(parameters, self.ADDFIELDS, context)
        launder = self.parameterAsBool(parameters, self.LAUNDER, context)
        launderstring = "-lco LAUNDER=NO"
        index = self.parameterAsBool(parameters, self.INDEX, context)
        indexstring = "-lco SPATIAL_INDEX=OFF"
        skipfailures = self.parameterAsBool(parameters, self.SKIPFAILURES, context)
        promotetomulti = self.parameterAsBool(parameters, self.PROMOTETOMULTI, context)
        precision = self.parameterAsBool(parameters, self.PRECISION, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)

        arguments = []
        arguments.append('-progress')
        arguments.append('--config PG_USE_COPY YES')
        if shapeEncoding:
            arguments.append('--config')
            arguments.append('SHAPE_ENCODING')
            arguments.append('"' + shapeEncoding + '"')
        arguments.append('-f')
        arguments.append('PostgreSQL')
        arguments.append('PG:"')
        for token in uri.connectionInfo(executing).split(' '):
            arguments.append(token)
        arguments.append('active_schema={}'.format(schema or 'public'))
        arguments.append('"')
        arguments.append(dimstring)
        arguments.append(ogrLayer)
        arguments.append(layername)
        if index:
            arguments.append(indexstring)
        if launder:
            arguments.append(launderstring)
        if append:
            arguments.append('-append')
        if addfields:
            arguments.append('-addfields')
        if overwrite:
            arguments.append('-overwrite')
        if len(self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)]) > 0:
            arguments.append('-nlt')
            arguments.append(self.GEOMTYPE[self.parameterAsEnum(parameters, self.GTYPE, context)])
        if len(geocolumn) > 0:
            arguments.append(geocolumnstring)
        if len(pk) > 0:
            arguments.append(pkstring)
        elif primary_key is not None:
            arguments.append("-lco FID=" + primary_key)
        if len(table) == 0:
            table = layername.lower()
        if schema:
            table = '{}.{}'.format(schema, table)
        arguments.append('-nln')
        arguments.append(table)
        if ssrs.isValid():
            arguments.append('-s_srs')
            arguments.append(GdalUtils.gdal_crs_string(ssrs))
        if tsrs.isValid():
            arguments.append('-t_srs')
            arguments.append(GdalUtils.gdal_crs_string(tsrs))
        if asrs.isValid():
            arguments.append('-a_srs')
            arguments.append(GdalUtils.gdal_crs_string(asrs))
        if not spat.isNull():
            arguments.append('-spat')
            arguments.append(spat.xMinimum())
            arguments.append(spat.yMinimum())
            arguments.append(spat.xMaximum())
            arguments.append(spat.yMaximum())
            if clip:
                arguments.append('-clipsrc spat_extent')
        if skipfailures:
            arguments.append('-skipfailures')
        if where:
            arguments.append(wherestring)
        if len(simplify) > 0:
            arguments.append('-simplify')
            arguments.append(simplify)
        if len(segmentize) > 0:
            arguments.append('-segmentize')
            arguments.append(segmentize)
        if len(gt) > 0:
            arguments.append('-gt')
            arguments.append(gt)
        if promotetomulti:
            arguments.append('-nlt PROMOTE_TO_MULTI')
        if precision is False:
            arguments.append('-lco PRECISION=NO')
        if len(options) > 0:
            arguments.append(options)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        return commands

    def commandName(self):
        return "ogr2ogr"
