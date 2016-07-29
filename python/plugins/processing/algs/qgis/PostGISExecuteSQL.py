# -*- coding: utf-8 -*-

"""
***************************************************************************
    PostGISExecuteSQL.py
    ---------------------
    Date                 : October 2012
    Copyright            : (C) 2012 by Victor Olaya and Carterix Geomatics
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

__author__ = 'Victor Olaya, Carterix Geomatics'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya, Carterix Geomatics'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QSettings

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterString
from processing.tools import postgis


class PostGISExecuteSQL(GeoAlgorithm):

    DATABASE = 'DATABASE'
    SQL = 'SQL'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('PostGIS execute SQL')
        self.group, self.i18n_group = self.trAlgorithm('Database')
        self.addParameter(ParameterString(self.DATABASE, self.tr('Database')))
        self.addParameter(ParameterString(self.SQL, self.tr('SQL query'), '', True))

    def processAlgorithm(self, progress):
        connection = self.getParameterValue(self.DATABASE)
        self.db = postgis.GeoDB.from_name(connection)
        sql = self.getParameterValue(self.SQL).replace('\n', ' ')
        try:
            self.db._exec_sql_and_commit(unicode(sql))
        except postgis.DbError as e:
            raise GeoAlgorithmExecutionException(
                self.tr('Error executing SQL:\n%s') % unicode(e))
