# -*- coding: utf-8 -*-

"""
***************************************************************************
    SpatialiteExecuteSQL.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
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

__author__ = 'Mathieu Pellerin'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Mathieu Pellerin'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.tools import spatialite

from qgis.core import (QgsApplication,
                       QgsDataSourceUri)


class SpatialiteExecuteSQL(QgisAlgorithm):

    DATABASE = 'DATABASE'
    SQL = 'SQL'

    def group(self):
        return self.tr('Database')

    def __init__(self):
        super().__init__()
        self.addParameter(ParameterVector(self.DATABASE, self.tr('File Database'), False, False))
        self.addParameter(ParameterString(self.SQL, self.tr('SQL query'), '', True))

    def name(self):
        return 'spatialiteexecutesql'

    def displayName(self):
        return self.tr('Spatialite execute SQL')

    def processAlgorithm(self, parameters, context, feedback):
        database = self.getParameterValue(self.DATABASE)
        uri = QgsDataSourceUri(database)
        if uri.database() is '':
            if '|layerid' in database:
                database = database[:database.find('|layerid')]
            uri = QgsDataSourceUri('dbname=\'%s\'' % (database))
        self.db = spatialite.GeoDB(uri)
        sql = self.getParameterValue(self.SQL).replace('\n', ' ')
        try:
            self.db._exec_sql_and_commit(str(sql))
        except spatialite.DbError as e:
            raise GeoAlgorithmExecutionException(
                self.tr('Error executing SQL:\n{0}').format(str(e)))
