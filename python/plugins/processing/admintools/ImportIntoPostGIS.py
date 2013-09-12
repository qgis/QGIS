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

import os
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterVector import ParameterVector
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.parameters.ParameterString import ParameterString
from processing.admintools import postgis_utils

class ImportIntoPostGIS(GeoAlgorithm):

    DATABASE = "DATABASE"
    TABLENAME = "TABLENAME"
    SCHEMA = "SCHEMA"
    INPUT = "INPUT"
    OVERWRITE = "OVERWRITE"
    CREATEINDEX = "CREATEINDEX"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/postgis.png")

    def processAlgorithm(self, progress):
        connection = self.getParameterValue(self.DATABASE)
        schema = self.getParameterValue(self.SCHEMA)
        overwrite = self.getParameterValue(self.OVERWRITE)
        createIndex = self.getParameterValue(self.CREATEINDEX)
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

        table = self.getParameterValue(self.TABLENAME);
        table.replace(" ", "")
        providerName = "postgres"

        try:
            db = postgis_utils.GeoDB(host=host, port=port, dbname=database, user=username, passwd=password)
        except postgis_utils.DbError, e:
            raise GeoAlgorithmExecutionException("Couldn't connect to database:\n"+e.message)

        uri = QgsDataSourceURI()
        uri.setConnection(host, str(port), database, username, password)
        uri.setDataSource(schema, table, "the_geom", "")

        options = {}
        if overwrite:
            options['overwrite'] = True

        layerUri = self.getParameterValue(self.INPUT);
        layer = QGisLayers.getObjectFromUri(layerUri)
        ret, errMsg = QgsVectorLayerImport.importLayer(layer, uri.uri(), providerName, self.crs, False, False, options)
        if ret != 0:
            raise GeoAlgorithmExecutionException(u"Error importing to PostGIS\n%s" %  errMsg)

        if createIndex:
            db.create_spatial_index(table, schema, "the_geom")

        db.vacuum_analyze(table, schema)

    def defineCharacteristics(self):
        self.name = "Import into PostGIS"
        self.group = "PostGIS management tools"
        self.addParameter(ParameterVector(self.INPUT, "Layer to import"))
        self.addParameter(ParameterString(self.DATABASE, "Database (connection name)"))
        self.addParameter(ParameterString(self.TABLENAME, "Table to import to"))
        self.addParameter(ParameterBoolean(self.OVERWRITE, "Overwrite", True))
        self.addParameter(ParameterBoolean(self.CREATEINDEX, "Create spatial index", True))





