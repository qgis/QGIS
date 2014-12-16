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

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterExtent

from processing.tools.system import *

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrToPostGis(OgrAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    GTYPE = 'GTYPE'
    GEOMTYPE = ['','NONE','GEOMETRY','POINT','LINESTRING','POLYGON','GEOMETRYCOLLECTION','MULTIPOINT','MULTIPOLYGON','MULTILINESTRING']
    S_SRS = 'S_SRS'
    T_SRS = 'T_SRS'
    HOST = 'HOST'
    PORT= 'PORT'
    USER = 'USER'
    DBNAME = 'DBNAME'
    PASSWORD = 'PASSWORD'
    SCHEMA = 'SCHEMA'
    TABLE = 'TABLE'
    PK = 'PK'
    GEOCOLUMN = 'GEOCOLUMN'
    DIM = 'DIM'
    DIMLIST = ['2','3']
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
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name = 'Import Vector into PostGIS database (new connection)'
        self.group = '[OGR] Miscellaneous'
        self.addParameter(ParameterVector(self.INPUT_LAYER, 'Input layer',
                          [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterSelection(self.GTYPE, 'Output geometry type',self.GEOMTYPE, 5))
        self.addParameter(ParameterCrs(self.S_SRS, 'Input CRS (EPSG Code)',
                          'EPSG:4326'))
        self.addParameter(ParameterCrs(self.T_SRS, 'Output CRS (EPSG Code)',
                          'EPSG:4326'))
        self.addParameter(ParameterString(self.HOST, 'Host',
                          'localhost', optional=False))
        self.addParameter(ParameterString(self.PORT, 'Port',
                          '5432', optional=False))
        self.addParameter(ParameterString(self.USER, 'Username',
                          '', optional=False))
        self.addParameter(ParameterString(self.DBNAME, 'Database Name',
                          '', optional=False))
        self.addParameter(ParameterString(self.PASSWORD, 'Password',
                          '', optional=False))
        self.addParameter(ParameterString(self.SCHEMA, 'Schema name',
                          'public', optional=True))
        self.addParameter(ParameterString(self.TABLE, 'Table name, leave blank to use input name',
                          '', optional=True))
        self.addParameter(ParameterString(self.PK, 'Primary Key',
                          'id', optional=True))
        self.addParameter(ParameterString(self.GEOCOLUMN, 'Geometry column name',
                          'geom', optional=True))
        self.addParameter(ParameterSelection(self.DIM, 'Vector dimensions',self.DIMLIST, 0))
        self.addParameter(ParameterString(self.SIMPLIFY, 'Distance tolerance for simplification',
                          '', optional=True))
        self.addParameter(ParameterString(self.SEGMENTIZE, 'Maximum distance between 2 nodes (densification)',
                          '', optional=True))
        self.addParameter(ParameterExtent(self.SPAT,
                          'Select features by extent (defined in input layer CRS)'))
        self.addParameter(ParameterBoolean(self.CLIP,
                          'Clip the input layer using the above (rectangle) extent', False))
        self.addParameter(ParameterString(self.WHERE, 'Select features using a SQL "WHERE" statement (Ex: column="value")',
                          '', optional=True))
        self.addParameter(ParameterString(self.GT, 'Group "n" features per transaction (Default: 20000)',
                          '', optional=True))
        self.addParameter(ParameterBoolean(self.OVERWRITE,
                          'Overwrite existing table?', True))
        self.addParameter(ParameterBoolean(self.APPEND,
                          'Append to existing table?', False))
        self.addParameter(ParameterBoolean(self.ADDFIELDS,
                          'Append and add new fields to existing table?', False))
        self.addParameter(ParameterBoolean(self.LAUNDER,
                          'Do not launder columns/table name/s?', False))
        self.addParameter(ParameterBoolean(self.INDEX,
                          'Do not create Spatial Index?', False))
        self.addParameter(ParameterBoolean(self.SKIPFAILURES,
                          'Continue after a failure, skipping the failed feature', False))
        self.addParameter(ParameterString(self.OPTIONS, 'Additional creation options',
                          '', optional=True))

    def processAlgorithm(self, progress):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)
        ssrs = unicode(self.getParameterValue(self.S_SRS))
        tsrs = unicode(self.getParameterValue(self.T_SRS))
        host = unicode(self.getParameterValue(self.HOST))
        port = unicode(self.getParameterValue(self.PORT))
        user = unicode(self.getParameterValue(self.USER))
        dbname = unicode(self.getParameterValue(self.DBNAME))
        password = unicode(self.getParameterValue(self.PASSWORD))
        schema = unicode(self.getParameterValue(self.SCHEMA))
        schemastring = "-lco SCHEMA="+schema
        table = unicode(self.getParameterValue(self.TABLE))
        pk = unicode(self.getParameterValue(self.PK))
        pkstring = "-lco FID="+pk
        geocolumn = unicode(self.getParameterValue(self.GEOCOLUMN))
        geocolumnstring = "-lco GEOMETRY_NAME="+geocolumn
        dim = self.DIMLIST[self.getParameterValue(self.DIM)]
        dimstring = "-lco DIM="+dim
        simplify = unicode(self.getParameterValue(self.SIMPLIFY))
        segmentize = unicode(self.getParameterValue(self.SEGMENTIZE))
        spat = self.getParameterValue(self.SPAT)
        ogrspat = self.ogrConnectionString(spat)
        clip = self.getParameterValue(self.CLIP)
        where = unicode(self.getParameterValue(self.WHERE))
        wherestring = "-where '"+where+"'"
        gt = unicode(self.getParameterValue(self.GT))
        overwrite = self.getParameterValue(self.OVERWRITE)
        append = self.getParameterValue(self.APPEND)
        addfields = self.getParameterValue(self.ADDFIELDS)
        launder = self.getParameterValue(self.LAUNDER)
        launderstring = "-lco LAUNDER=NO"
        index = self.getParameterValue(self.INDEX)
        indexstring = "-lco SPATIAL_INDEX=OFF"
        skipfailures = self.getParameterValue(self.SKIPFAILURES)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append('-progress')
        arguments.append('--config PG_USE_COPY YES')
        arguments.append('-f')
        arguments.append('PostgreSQL')
        arguments.append('PG:"host=')
        arguments.append(host)
        arguments.append('port=')
        arguments.append(port)
        arguments.append('user=')
        arguments.append(user)
        arguments.append('dbname=')
        arguments.append(dbname)
        arguments.append('password=')
        arguments.append(password)
        arguments.append('"')
        arguments.append(dimstring)
        arguments.append(ogrLayer)
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
        if len(schema) > 0:
            arguments.append(schemastring)
        if len(geocolumn) > 0:
            arguments.append(geocolumnstring)
        if len(pk) > 0:
            arguments.append(pkstring)
        if len(table) > 0:
            arguments.append('-nln')
            arguments.append(table)
        if len(ssrs) > 0:
            arguments.append('-s_srs')
            arguments.append(ssrs)
        if len(tsrs) > 0:
            arguments.append('-t_srs')
            arguments.append(tsrs)
        if len(spat) > 0:
            regionCoords = ogrspat.split(',')
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
        if len(options) > 0:
            arguments.append(options)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)