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

from PyQt4.QtCore import QSettings

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterTableField

from processing.tools.system import isWindows

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrTableToPostGisList(OgrAlgorithm):

    DATABASE = 'DATABASE'
    INPUT_LAYER = 'INPUT_LAYER'
    HOST = 'HOST'
    PORT= 'PORT'
    USER = 'USER'
    DBNAME = 'DBNAME'
    PASSWORD = 'PASSWORD'
    SCHEMA = 'SCHEMA'
    TABLE = 'TABLE'
    PK = 'PK'
    PRIMARY_KEY = 'PRIMARY_KEY'
    WHERE = 'WHERE'
    GT = 'GT'
    OVERWRITE = 'OVERWRITE'
    APPEND = 'APPEND'
    ADDFIELDS = 'ADDFIELDS'
    LAUNDER = 'LAUNDER'
    SKIPFAILURES = 'SKIPFAILURES'
    PRECISION = 'PRECISION'
    OPTIONS = 'OPTIONS'

    def dbConnectionNames(self):
        settings = QSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        return settings.childGroups()

    def defineCharacteristics(self):
        self.name = 'Import layer/table as geometryless table into PostgreSQL database'
        self.group = '[OGR] Miscellaneous'
        self.DB_CONNECTIONS = self.dbConnectionNames()
        self.addParameter(ParameterSelection(self.DATABASE,
            self.tr('Database (connection name)'), self.DB_CONNECTIONS))
        self.addParameter(ParameterTable(self.INPUT_LAYER,
            self.tr('Input layer')))
        self.addParameter(ParameterString(self.SCHEMA,
            self.tr('Schema name'), 'public', optional=True))
        self.addParameter(ParameterString(self.TABLE,
            self.tr('Table name, leave blank to use input name'),
            '', optional=True))
        self.addParameter(ParameterString(self.PK,
            self.tr('Primary key'), 'id', optional=True))
        self.addParameter(ParameterTableField(self.PRIMARY_KEY,
            self.tr('Primary key (existing field, used if the above option is left empty)'),
            self.INPUT_LAYER, optional=True))
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
        self.addParameter(ParameterBoolean(self.SKIPFAILURES,
            self.tr('Continue after a failure, skipping the failed record'),
            False))
        self.addParameter(ParameterBoolean(self.PRECISION,
            self.tr('Keep width and precision of input attributes'),
            True))
        self.addParameter(ParameterString(self.OPTIONS,
            self.tr('Additional creation options'), '', optional=True))

    def getConsoleCommands(self):
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
        primary_key = self.getParameterValue(self.PRIMARY_KEY)
        where = unicode(self.getParameterValue(self.WHERE))
        wherestring = '-where "'+where+'"'
        gt = unicode(self.getParameterValue(self.GT))
        overwrite = self.getParameterValue(self.OVERWRITE)
        append = self.getParameterValue(self.APPEND)
        addfields = self.getParameterValue(self.ADDFIELDS)
        launder = self.getParameterValue(self.LAUNDER)
        launderstring = "-lco LAUNDER=NO"
        skipfailures = self.getParameterValue(self.SKIPFAILURES)
        precision = self.getParameterValue(self.PRECISION)
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
        if len(dbname) > 0:
            arguments.append('dbname=' + dbname)
        if len(password) > 0:
            arguments.append('password=' + password)
        arguments.append('user=' + user + '"')
        arguments.append(ogrLayer)
        arguments.append('-nlt NONE')
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
        elif primary_key is not None:
            arguments.append("-lco FID="+primary_key)
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
        if not precision:
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