# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrtabletopostgislist.py
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
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterExtent
from processing.tools import dataobjects
from processing.algs.qgis import postgis_utils

from processing.tools.system import *

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrTableToPostGisList(OgrAlgorithm):

    DATABASE = 'DATABASE'
    INPUT_LAYER = 'INPUT_LAYER'
    GTYPE = 'GTYPE'
    GEOMTYPE = ['NONE']
    HOST = 'HOST'
    PORT= 'PORT'
    USER = 'USER'
    DBNAME = 'DBNAME'
    PASSWORD = 'PASSWORD'
    SCHEMA = 'SCHEMA'
    TABLE = 'TABLE'
    PK = 'PK'
    WHERE = 'WHERE'
    GT = 'GT'
    OVERWRITE = 'OVERWRITE'
    APPEND = 'APPEND'
    ADDFIELDS = 'ADDFIELDS'
    LAUNDER = 'LAUNDER'
    SKIPFAILURES = 'SKIPFAILURES'
    OPTIONS = 'OPTIONS'

    def dbConnectionNames(self):
        settings = QSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        return settings.childGroups()

    def defineCharacteristics(self):
        self.name = 'Import Geometryless Table into PostGIS database (available connections)'
        self.group = '[OGR] Miscellaneous'
        self.DB_CONNECTIONS = self.dbConnectionNames()
        self.addParameter(ParameterSelection(self.DATABASE,
            self.tr('Database (connection name)'), self.DB_CONNECTIONS))
        self.addParameter(ParameterTable(self.INPUT_LAYER,
            self.tr('Input layer')))
        self.addParameter(ParameterSelection(self.GTYPE,
            self.tr('Output geometry type'), self.GEOMTYPE, 0))
        self.addParameter(ParameterString(self.SCHEMA,
            self.tr('Schema name'), 'public', optional=True))
        self.addParameter(ParameterString(self.TABLE,
            self.tr('Table name, leave blank to use input name'),
            '', optional=True))
        self.addParameter(ParameterString(self.PK,
            self.tr('Primary key'), 'id', optional=True))
        self.addParameter(ParameterString(self.WHERE,
            self.tr('Select features using a SQL "WHERE" statement (Ex: column="value")'),
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
        self.addParameter(ParameterBoolean(self.SKIPFAILURES,
            self.tr('Continue after a failure, skipping the failed record'),
            False))
        self.addParameter(ParameterString(self.OPTIONS,
            self.tr('Additional creation options'), '', optional=True))

    def processAlgorithm(self, progress):
        connection = self.DB_CONNECTIONS[self.getParameterValue(self.DATABASE)]
        settings = QSettings()
        mySettings = '/PostgreSQL/connections/' + connection
        dbname = settings.value(mySettings + '/database')
        user = settings.value(mySettings + '/username')
        host = settings.value(mySettings + '/host')
        port = settings.value(mySettings + '/port')
        password = settings.value(mySettings + '/password')
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)[1:-1]
        schema = unicode(self.getParameterValue(self.SCHEMA))
        schemastring = "-lco SCHEMA="+schema
        table = unicode(self.getParameterValue(self.TABLE))
        pk = unicode(self.getParameterValue(self.PK))
        pkstring = "-lco FID="+pk
        where = unicode(self.getParameterValue(self.WHERE))
        wherestring = "-where '"+where+"'"
        gt = unicode(self.getParameterValue(self.GT))
        overwrite = self.getParameterValue(self.OVERWRITE)
        append = self.getParameterValue(self.APPEND)
        addfields = self.getParameterValue(self.ADDFIELDS)
        launder = self.getParameterValue(self.LAUNDER)
        launderstring = "-lco LAUNDER=NO"
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
        arguments.append(ogrLayer)
        arguments.append(self.ogrLayerName(inLayer))
        if launder:
            arguments.append(launderstring)
        if append:
            arguments.append('-append')
        if addfields:
            arguments.append('-addfields')
        if overwrite:
            arguments.append('-overwrite')
        if len(schema) > 0:
            arguments.append(schemastring)
        if len(pk) > 0:
            arguments.append(pkstring)
        if len(table) > 0:
            arguments.append('-nln')
            arguments.append(table)
        if skipfailures:
            arguments.append('-skipfailures')
        if where:
            arguments.append(wherestring)
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