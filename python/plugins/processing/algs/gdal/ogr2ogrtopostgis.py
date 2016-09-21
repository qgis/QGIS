# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrtopostgis.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterTableField

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows
from processing.tools.vector import ogrConnectionString, ogrLayerName


class Ogr2OgrToPostGis(GdalAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    SHAPE_ENCODING = 'SHAPE_ENCODING'
    GTYPE = 'GTYPE'
    GEOMTYPE = ['', 'NONE', 'GEOMETRY', 'POINT', 'LINESTRING', 'POLYGON', 'GEOMETRYCOLLECTION', 'MULTIPOINT', 'MULTIPOLYGON', 'MULTILINESTRING']
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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Import Vector into PostGIS database (new connection)')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Miscellaneous')
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterString(self.SHAPE_ENCODING,
                                          self.tr('Shape encoding'), "", optional=True))
        self.addParameter(ParameterSelection(self.GTYPE,
                                             self.tr('Output geometry type'), self.GEOMTYPE, 0))
        self.addParameter(ParameterCrs(self.A_SRS,
                                       self.tr('Assign an output CRS'), '', optional=True))
        self.addParameter(ParameterCrs(self.T_SRS,
                                       self.tr('Reproject to this CRS on output '), '', optional=True))
        self.addParameter(ParameterCrs(self.S_SRS,
                                       self.tr('Override source CRS'), '', optional=True))
        self.addParameter(ParameterString(self.HOST,
                                          self.tr('Host'), 'localhost', optional=False))
        self.addParameter(ParameterString(self.PORT,
                                          self.tr('Port'), '5432', optional=False))
        self.addParameter(ParameterString(self.USER,
                                          self.tr('Username'), '', optional=False))
        self.addParameter(ParameterString(self.DBNAME,
                                          self.tr('Database name'), '', optional=False))
        self.addParameter(ParameterString(self.PASSWORD,
                                          self.tr('Password'), '', optional=False))
        self.addParameter(ParameterString(self.SCHEMA,
                                          self.tr('Schema name'), 'public', optional=True))
        self.addParameter(ParameterString(self.TABLE,
                                          self.tr('Table name, leave blank to use input name'),
                                          '', optional=True))
        self.addParameter(ParameterString(self.PK,
                                          self.tr('Primary key (new field)'), 'id', optional=True))
        self.addParameter(ParameterTableField(self.PRIMARY_KEY,
                                              self.tr('Primary key (existing field, used if the above option is left empty)'), self.INPUT_LAYER, optional=True))
        self.addParameter(ParameterString(self.GEOCOLUMN,
                                          self.tr('Geometry column name'), 'geom', optional=True))
        self.addParameter(ParameterSelection(self.DIM,
                                             self.tr('Vector dimensions'), self.DIMLIST, 0))
        self.addParameter(ParameterString(self.SIMPLIFY,
                                          self.tr('Distance tolerance for simplification'),
                                          '', optional=True))
        self.addParameter(ParameterString(self.SEGMENTIZE,
                                          self.tr('Maximum distance between 2 nodes (densification)'),
                                          '', optional=True))
        self.addParameter(ParameterExtent(self.SPAT,
                                          self.tr('Select features by extent (defined in input layer CRS)')))
        self.addParameter(ParameterBoolean(self.CLIP,
                                           self.tr('Clip the input layer using the above (rectangle) extent'),
                                           False))
        self.addParameter(ParameterString(self.WHERE,
                                          self.tr('Select features using a SQL "WHERE" statement (Ex: column=\'value\')'),
                                          '', optional=True))
        self.addParameter(ParameterString(self.GT,
                                          self.tr('Group N features per transaction (Default: 20000)'),
                                          '', optional=True))
        self.addParameter(ParameterBoolean(self.OVERWRITE,
                                           self.tr('Overwrite existing table'), True))
        self.addParameter(ParameterBoolean(self.APPEND,
                                           self.tr('Append to existing table'), False))
        self.addParameter(ParameterBoolean(self.ADDFIELDS,
                                           self.tr('Append and add new fields to existing table'), False))
        self.addParameter(ParameterBoolean(self.LAUNDER,
                                           self.tr('Do not launder columns/table names'), False))
        self.addParameter(ParameterBoolean(self.INDEX,
                                           self.tr('Do not create spatial index'), False))
        self.addParameter(ParameterBoolean(self.SKIPFAILURES,
                                           self.tr('Continue after a failure, skipping the failed feature'), False))
        self.addParameter(ParameterBoolean(self.PROMOTETOMULTI,
                                           self.tr('Promote to Multipart'),
                                           True))
        self.addParameter(ParameterBoolean(self.PRECISION,
                                           self.tr('Keep width and precision of input attributes'),
                                           True))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'), '', optional=True))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = ogrConnectionString(inLayer)[1:-1]
        shapeEncoding = self.getParameterValue(self.SHAPE_ENCODING)
        ssrs = str(self.getParameterValue(self.S_SRS))
        tsrs = str(self.getParameterValue(self.T_SRS))
        asrs = str(self.getParameterValue(self.A_SRS))
        host = str(self.getParameterValue(self.HOST))
        port = str(self.getParameterValue(self.PORT))
        user = str(self.getParameterValue(self.USER))
        dbname = str(self.getParameterValue(self.DBNAME))
        password = str(self.getParameterValue(self.PASSWORD))
        schema = str(self.getParameterValue(self.SCHEMA))
        table = str(self.getParameterValue(self.TABLE))
        pk = str(self.getParameterValue(self.PK))
        pkstring = "-lco FID=" + pk
        primary_key = self.getParameterValue(self.PRIMARY_KEY)
        geocolumn = str(self.getParameterValue(self.GEOCOLUMN))
        geocolumnstring = "-lco GEOMETRY_NAME=" + geocolumn
        dim = self.DIMLIST[self.getParameterValue(self.DIM)]
        dimstring = "-lco DIM=" + dim
        simplify = str(self.getParameterValue(self.SIMPLIFY))
        segmentize = str(self.getParameterValue(self.SEGMENTIZE))
        spat = self.getParameterValue(self.SPAT)
        clip = self.getParameterValue(self.CLIP)
        where = str(self.getParameterValue(self.WHERE))
        wherestring = '-where "' + where + '"'
        gt = str(self.getParameterValue(self.GT))
        overwrite = self.getParameterValue(self.OVERWRITE)
        append = self.getParameterValue(self.APPEND)
        addfields = self.getParameterValue(self.ADDFIELDS)
        launder = self.getParameterValue(self.LAUNDER)
        launderstring = "-lco LAUNDER=NO"
        index = self.getParameterValue(self.INDEX)
        indexstring = "-lco SPATIAL_INDEX=OFF"
        skipfailures = self.getParameterValue(self.SKIPFAILURES)
        promotetomulti = self.getParameterValue(self.PROMOTETOMULTI)
        precision = self.getParameterValue(self.PRECISION)
        options = str(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append('-progress')
        arguments.append('--config PG_USE_COPY YES')
        if len(shapeEncoding) > 0:
            arguments.append('--config')
            arguments.append('SHAPE_ENCODING')
            arguments.append('"' + shapeEncoding + '"')
        arguments.append('-f')
        arguments.append('PostgreSQL')
        arguments.append('PG:"host=' + host)
        arguments.append('port=' + port)
        if len(dbname) > 0:
            arguments.append('dbname=' + dbname)
        if len(password) > 0:
            arguments.append('password=' + password)
        if len(schema) > 0:
            arguments.append('active_schema=' + schema)
        else:
            arguments.append('active_schema=public')
        arguments.append('user=' + user + '"')
        arguments.append(dimstring)
        arguments.append(ogrLayer)
        arguments.append(ogrLayerName(inLayer))
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
        if len(self.GEOMTYPE[self.getParameterValue(self.GTYPE)]) > 0:
            arguments.append('-nlt')
            arguments.append(self.GEOMTYPE[self.getParameterValue(self.GTYPE)])
        if len(geocolumn) > 0:
            arguments.append(geocolumnstring)
        if len(pk) > 0:
            arguments.append(pkstring)
        elif primary_key is not None:
            arguments.append("-lco FID=" + primary_key)
        if len(table) == 0:
            table = ogrLayerName(inLayer).lower()
        if schema:
            table = '{}.{}'.format(schema, table)
        arguments.append('-nln')
        arguments.append(table)
        if len(ssrs) > 0:
            arguments.append('-s_srs')
            arguments.append(ssrs)
        if len(tsrs) > 0:
            arguments.append('-t_srs')
            arguments.append(tsrs)
        if len(asrs) > 0:
            arguments.append('-a_srs')
            arguments.append(asrs)
        if len(spat) > 0:
            regionCoords = spat.split(',')
            arguments.append('-spat')
            arguments.append(regionCoords[0])
            arguments.append(regionCoords[2])
            arguments.append(regionCoords[1])
            arguments.append(regionCoords[3])
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
