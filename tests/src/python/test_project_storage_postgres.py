# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres project storage.

Note: to prepare the DB, you need to run the sql files specified in
tests/testdata/provider/testdata_pg.sh

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next

__author__ = 'Martin Dobias'
__date__ = '2018-03-29'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA
import psycopg2

import os
import time

from qgis.core import (
    QgsApplication,
    QgsDataSourceUri,
    QgsVectorLayer,
    QgsProject,
)
from PyQt5.QtCore import QDateTime, QUrl, QUrlQuery
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from test_project_storage_base import TestPyQgsProjectStorageBase

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProjectStoragePostgres(TestPyQgsProjectStorageBase, unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        super().setUpClass()

        cls.dbconn = 'service=qgis_test'
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        cls.ds_uri = QgsDataSourceUri(cls.dbconn)

        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert cls.vl.isValid()
        cls.con = psycopg2.connect(cls.dbconn)

        cls.schema = 'qgis_test'
        cls.provider = 'postgres'
        cls.project_storage_type = 'postgresql'

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def execSQLCommand(self, sql):
        self.assertTrue(self.con)
        cur = self.con.cursor()
        self.assertTrue(cur)
        cur.execute(sql)
        cur.close()
        self.con.commit()

    def dropProjectsTable(self):
        self.execSQLCommand("DROP TABLE IF EXISTS qgis_test.qgis_projects;")

    def encode_uri(self, ds_uri, schema_name, project_name=None):
        u = QUrl()
        urlQuery = QUrlQuery()

        u.setScheme("postgresql")
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
