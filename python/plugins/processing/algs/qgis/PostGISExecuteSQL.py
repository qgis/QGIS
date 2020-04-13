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

from qgis.core import (
    QgsProcessingException,
    QgsProcessingParameterString,
    QgsProcessingParameterProviderConnection,
    QgsProviderRegistry,
    QgsProviderConnectionException
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PostGISExecuteSQL(QgisAlgorithm):

    DATABASE = 'DATABASE'
    SQL = 'SQL'

    def group(self):
        return self.tr('Database')

    def groupId(self):
        return 'database'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        db_param = QgsProcessingParameterProviderConnection(
            self.DATABASE,
            self.tr('Database (connection name)'), 'postgres')
        self.addParameter(db_param)
        self.addParameter(QgsProcessingParameterString(self.SQL, self.tr('SQL query'), multiLine=True))

    def name(self):
        return 'postgisexecutesql'

    def displayName(self):
        return self.tr('PostgreSQL execute SQL')

    def shortDescription(self):
        return self.tr('Executes a SQL command on a PostgreSQL database')

    def tags(self):
        return self.tr('postgis,database').split(',')

    def processAlgorithm(self, parameters, context, feedback):
        connection_name = self.parameterAsConnectionName(parameters, self.DATABASE, context)

        # resolve connection details to uri
        try:
            md = QgsProviderRegistry.instance().providerMetadata('postgres')
            conn = md.createConnection(connection_name)
        except QgsProviderConnectionException:
            raise QgsProcessingException(self.tr('Could not retrieve connection details for {}').format(connection_name))

        sql = self.parameterAsString(parameters, self.SQL, context).replace('\n', ' ')
        try:
            conn.executeSql(sql)
        except QgsProviderConnectionException as e:
            raise QgsProcessingException(self.tr('Error executing SQL:\n{0}').format(e))

        return {}
