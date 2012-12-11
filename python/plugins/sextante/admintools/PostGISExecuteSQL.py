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
    TABLENAME = "TABLENAME"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/postgis.png")

    def processAlgorithm(self, progress):

        connection = self.getParameterValue(self.DATABASE)
        settings = QSettings()
        mySettings = "/PostgreSQL/connections/"+ connection
        database = settings.value(mySettings+"/database").toString()
        username = settings.value(mySettings+"/username").toString()
        host = settings.value(mySettings+"/host").toString()
        port = settings.value(mySettings+"/port").toString()
        password = settings.value(mySettings+"/password").toString()

        # connect to DB
        try:
            self.db = postgis_utils.GeoDB(host=host, port=port, dbname=database, user=username, passwd=password)
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Couldn't connect to database:\n"+e.message)

        ##  Set up sql statement for geoprocess
        newTbl = self.getParameterValue(self.TABLENAME);
        newTbl.replace(" ", "")
        sqlNewTbl = str("CREATE TABLE ") + str(newTbl) + " as "
        txtSQL = self.getParameterValue(self.SQL)
        txtSQL =  sqlNewTbl + txtSQL
        sqlArray = txtSQL.split("\n")
        sqlString = ""
        for i in range(0,sqlArray.count()):
            sqlString = sqlString + sqlArray[i] + " "

        ##  Run query
        try:
            self.db._exec_sql_and_commit(str(sqlString))
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Couldn't connect to database:\n"+e.message)

        try: #first try
                dbGeoTbls = self.db.list_geotables(self.cmbSchema.currentText())
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Couldn't connect to database:\n"+e.message)

        for i in range(0,len(dbGeoTbls)):
            if dbGeoTbls[i][0]==newTbl:
                if dbGeoTbls[i][7]=="geometry":
                    geocol = str(dbGeoTbls[i][6])

        schName = self.cmbSchema.currentText()

        ##  Update the geometry_columns table
        geocolSQL = "SELECT ST_Dimension(" + geocol + "), ST_SRID(" + geocol + "), GeometryType(" + geocol + ") FROM " + newTbl + ";"
        c = self.db.con.cursor()
        self.db._exec_sql(c, str(geocolSQL))
        geomPar = c.fetchone()

        ##  Setup sql statement to update geometry_columns
        geocolupdate = "INSERT INTO geometry_columns "
        geocolupdate = geocolupdate + "(f_table_catalog, f_table_schema, f_table_name, f_geometry_column, coord_dimension, srid, type) VALUES"
        geocolupdate = geocolupdate + "('', 'public', '" + newTbl + "', '" + geocol + "', '" + str(geomPar[0]) + "', '" + str(geomPar[1]) + "', '" + str(geomPar[2])
        geocolupdate = geocolupdate + "');"
        self.db._exec_sql_and_commit(str(geocolupdate))

        ##  Add remaining constraints for dimension, geometry type and srid
        pkeySQL = "ALTER TABLE " + schName + "." + newTbl + " ADD CONSTRAINT " + newTbl + "_pkey PRIMARY KEY(pgid);"
        dimSQL = "ALTER TABLE " + schName + "." + newTbl + " ADD CONSTRAINT enforce_dims_" + geocol + " CHECK (ndims(" + geocol + ") = " + str(geomPar[0]) + ");"
        gtypSQL = "ALTER TABLE " + schName + "." + newTbl + " ADD CONSTRAINT enforce_geotype_" + geocol + " CHECK (geometrytype(" + geocol + ") = '" + str(geomPar[2]) + "'::text OR " + geocol + " IS NULL);"
        sridSQL = "ALTER TABLE " + schName + "." + newTbl + " ADD CONSTRAINT enforce_srid_" + geocol + " CHECK (srid(" + geocol + ") = " + str(geomPar[1]) + ");"

        try: #second try
                self.db._exec_sql_and_commit(str(pkeySQL))
                self.db._exec_sql_and_commit(str(dimSQL))
                self.db._exec_sql_and_commit(str(gtypSQL))
                self.db._exec_sql_and_commit(str(sridSQL))
        except postgis_utils.DbError, e:
                QMessageBox.critical(self, "error", "Couldn't connect to database:\n"+e.message)
                return

        #=======================================================================
        # ##  ##  Add resulting data set if Add layer checkbox is selected
        # if self.actionAddData.isChecked():
        #        ##    Set up data source and add to QGIS view
        #        uri = QgsDataSourceURI()
        #        uri.setConnection(str(self.db.host), str(self.db.port), str(self.db.dbname), str(self.db.user), str(self.db.passwd))
        #        uri.setDataSource(schName, newTbl, geocol)
        #        dbSchemas = self.db.list_schemas()
        #        for s in range(0,len(dbSchemas)):
        #                dbSchema = dbSchemas[s][1]
        #                if dbSchema==schName:
        #                        dbOwner = dbSchemas[s][2]
        #                        self.iface.addVectorLayer(uri.uri(), newTbl, dbOwner)
        #=======================================================================

    def defineCharacteristics(self):
        self.name = "PostGIS execute SQL"
        self.group = "PostGIS management tools"
        self.addParameter(ParameterString(self.DATABASE, "Database"))
        self.addParameter(ParameterString(self.TABLENAME, "Name for new table"))
        self.addParameter(ParameterString(self.SQL, "SQL query", True))



