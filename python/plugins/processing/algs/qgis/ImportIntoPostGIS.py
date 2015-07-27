# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportIntoPostGIS.py
    ---------------------
    Date                 : October 2012
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
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtCore import QSettings
from qgis.core import QgsDataSourceURI, QgsVectorLayerImport

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterTableField
from processing.tools import dataobjects
from processing.algs.qgis import postgis_utils


class ImportIntoPostGIS(GeoAlgorithm):

    DATABASE = 'DATABASE'
    TABLENAME = 'TABLENAME'
    SCHEMA = 'SCHEMA'
    INPUT = 'INPUT'
    OVERWRITE = 'OVERWRITE'
    CREATEINDEX = 'CREATEINDEX'
    GEOMETRY_COLUMN = 'GEOMETRY_COLUMN'
    LOWERCASE_NAMES = 'LOWERCASE_NAMES'
    DROP_STRING_LENGTH = 'DROP_STRING_LENGTH'
    PRIMARY_KEY = 'PRIMARY_KEY'

    def processAlgorithm(self, progress):
        connection = self.DB_CONNECTIONS[self.getParameterValue(self.DATABASE)]
        schema = self.getParameterValue(self.SCHEMA)
        overwrite = self.getParameterValue(self.OVERWRITE)
        createIndex = self.getParameterValue(self.CREATEINDEX)
        convertLowerCase = self.getParameterValue(self.LOWERCASE_NAMES)
        dropStringLength = self.getParameterValue(self.DROP_STRING_LENGTH)
        primaryKeyField = self.getParameterValue(self.PRIMARY_KEY)
        settings = QSettings()
        mySettings = '/PostgreSQL/connections/' + connection
        try:
            database = settings.value(mySettings + '/database')
            username = settings.value(mySettings + '/username')
            host = settings.value(mySettings + '/host')
            port = settings.value(mySettings + '/port', type=int)
            password = settings.value(mySettings + '/password')
        except Exception, e:
            raise GeoAlgorithmExecutionException(
                self.tr('Wrong database connection name: %s' % connection))

        layerUri = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(layerUri)

        table = self.getParameterValue(self.TABLENAME).strip()
        if table == '':
            table = layer.name().lower()
        table.replace(' ', '')
        providerName = 'postgres'

        try:
            db = postgis_utils.GeoDB(host=host, port=port, dbname=database,
                                     user=username, passwd=password)
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException(
                self.tr("Couldn't connect to database:\n%s" % e.message))

        geomColumn = self.getParameterValue(self.GEOMETRY_COLUMN)
        if not geomColumn:
            geomColumn = 'the_geom'

        options = {}
        if overwrite:
            options['overwrite'] = True
        if convertLowerCase:
            options['lowercaseFieldNames'] = True
            geomColumn = geomColumn.lower()
        if dropStringLength:
            options['dropStringConstraints'] = True

        #clear geometry column for non-geometry tables
        if not layer.hasGeometryType():
            geomColumn = None

        uri = QgsDataSourceURI()
        uri.setConnection(host, str(port), database, username, password)
        if primaryKeyField:
            uri.setDataSource(schema, table, geomColumn, '', primaryKeyField)
        else:
            uri.setDataSource(schema, table, geomColumn, '')

        (ret, errMsg) = QgsVectorLayerImport.importLayer(
            layer,
            uri.uri(),
            providerName,
            self.crs,
            False,
            False,
            options,
        )
        if ret != 0:
            raise GeoAlgorithmExecutionException(
                self.tr('Error importing to PostGIS\n%s' % errMsg))

        if geomColumn and createIndex:
            db.create_spatial_index(table, schema, geomColumn)

        db.vacuum_analyze(table, schema)

    def dbConnectionNames(self):
        settings = QSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        return settings.childGroups()

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Import into PostGIS')
        self.group, self.i18n_group = self.trAlgorithm('Database')
        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Layer to import')))

        self.DB_CONNECTIONS = self.dbConnectionNames()
        self.addParameter(ParameterSelection(self.DATABASE,
            self.tr('Database (connection name)'), self.DB_CONNECTIONS))
        self.addParameter(ParameterString(self.SCHEMA,
            self.tr('Schema (schema name)'), 'public'))
        self.addParameter(ParameterString(self.TABLENAME,
            self.tr('Table to import to (leave blank to use layer name)')))
        self.addParameter(ParameterTableField(self.PRIMARY_KEY,
            self.tr('Primary key field'), self.INPUT, optional=True))
        self.addParameter(ParameterString(self.GEOMETRY_COLUMN,
            self.tr('Geometry column'), 'geom'))
        self.addParameter(ParameterBoolean(self.OVERWRITE,
            self.tr('Overwrite'), True))
        self.addParameter(ParameterBoolean(self.CREATEINDEX,
            self.tr('Create spatial index'), True))
        self.addParameter(ParameterBoolean(self.LOWERCASE_NAMES,
            self.tr('Convert field names to lowercase'), True))
        self.addParameter(ParameterBoolean(self.DROP_STRING_LENGTH,
            self.tr('Drop length constraints on character fields'), False))
