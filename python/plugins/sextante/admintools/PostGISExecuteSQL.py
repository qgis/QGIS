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
from sextante.core.GeoAlgorithm import GeoAlgorithm

__author__ = 'Victor Olaya, Carterix Geomatics'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya, Carterix Geomatics'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.admintools import postgis_utils
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class PostGISExecuteSQL(GeoAlgorithm):

    DATABASE = "DATABASE"
    SQL = "SQL"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/postgis.png")

    def processAlgorithm(self, progress):

        connection = self.getParameterValue(self.DATABASE)
        settings = QSettings()
        mySettings = "/PostgreSQL/connections/"+ connection
        try:
            database = settings.value(mySettings+"/database")
            username = settings.value(mySettings+"/username")
            host = settings.value(mySettings+"/host")
            port = settings.value(mySettings+"/port", type = int)
            password = settings.value(mySettings+"/password")
        except Exception, e:
            raise GeoAlgorithmExecutionException("Wrong database connection name: " + connection)
        try:
            self.db = postgis_utils.GeoDB(host=host, port=port, dbname=database, user=username, passwd=password)
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Couldn't connect to database:\n"+e.message)

        sql = self.getParameterValue(self.SQL).replace("\n", " ")
        try:
            self.db._exec_sql_and_commit(str(sql))
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Error executing SQL:\n"+e.message)

    def defineCharacteristics(self):
        self.name = "PostGIS execute SQL"
        self.group = "PostGIS management tools"
        self.addParameter(ParameterString(self.DATABASE, "Database"))
        self.addParameter(ParameterString(self.SQL, "SQL query", "", True))



