# -*- coding: utf-8 -*-
"""QGIS Unit tests base for the database project storage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next

__author__ = 'Julien Cabieces'
__date__ = '2022-04-19'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

import os
import time

from qgis.core import (
    QgsApplication,
    QgsDataSourceUri,
    QgsVectorLayer,
    QgsProject,
)
from PyQt5.QtCore import QDateTime, QUrl, QUrlQuery
from qgis.PyQt.QtSql import QSqlDatabase, QSqlQuery
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from test_project_storage_base import TestPyQgsProjectStorageBase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProjectStorageOracle(TestPyQgsProjectStorageBase, unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        super().setUpClass()

        cls.dbconn = "host=localhost dbname=XEPDB1 port=1521 user='QGIS' password='qgis'"
        if 'QGIS_ORACLETEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_ORACLETEST_DB']
        cls.ds_uri = QgsDataSourceUri(cls.dbconn)

        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="QGIS"."SOME_DATA" (GEOM) sql=', 'test', 'oracle')
        assert cls.vl.isValid()

        cls.con = QSqlDatabase.addDatabase('QOCISPATIAL', "oracletest")
        cls.con.setDatabaseName('localhost/XEPDB1')
        if 'QGIS_ORACLETEST_DBNAME' in os.environ:
            cls.con.setDatabaseName(os.environ['QGIS_ORACLETEST_DBNAME'])
        cls.con.setUserName('QGIS')
        cls.con.setPassword('qgis')

        cls.schema = 'QGIS'
        cls.provider = 'oracle'
        cls.project_storage_type = 'oracle'

        assert cls.con.open()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def execSQLCommand(self, sql, ignore_errors=False):
        self.assertTrue(self.con)
        query = QSqlQuery(self.con)
        res = query.exec_(sql)
        if not ignore_errors:
            self.assertTrue(res, sql + ': ' + query.lastError().text())
        query.finish()

    def dropProjectsTable(self):
        self.execSQLCommand('DROP TABLE {}."qgis_projects"'.format(self.schema), True)

    def encode_uri(self, ds_uri, schema_name, project_name=None):
        u = QUrl()
        urlQuery = QUrlQuery()

        u.setScheme("oracle")
        u.setHost(ds_uri.host())
        if ds_uri.port() != '':
            u.setPort(int(ds_uri.port()))
        if ds_uri.username() != '':
            u.setUserName(ds_uri.username())
        if ds_uri.password() != '':
            u.setPassword(ds_uri.password())

        if ds_uri.service() != '':
            urlQuery.addQueryItem("service", ds_uri.service())
        if ds_uri.authConfigId() != '':
            urlQuery.addQueryItem("authcfg", ds_uri.authConfigId())
        if ds_uri.sslMode() != QgsDataSourceUri.SslPrefer:
            urlQuery.addQueryItem("sslmode", QgsDataSourceUri.encodeSslMode(ds_uri.sslMode()))

        urlQuery.addQueryItem("dbname", ds_uri.database())

        urlQuery.addQueryItem("schema", schema_name)
        if project_name:
            urlQuery.addQueryItem("project", project_name)

        u.setQuery(urlQuery)
        return str(u.toEncoded(), 'utf-8')


if __name__ == '__main__':
    unittest.main()
