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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import psycopg2

import os
import time

from qgis.core import (
    QgsApplication,
    QgsVectorLayer,
    QgsProject,
)
from PyQt5.QtCore import QDateTime
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProjectStoragePostgres(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert cls.vl.isValid()
        cls.con = psycopg2.connect(cls.dbconn)

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

    def testSaveLoadProject(self):

        # TODO: respect QGIS_PGTEST_DB
        schema_uri = "postgresql:///?dbname=qgis_test&schema=qgis_test"
        project_uri = "postgresql:///?dbname=qgis_test&schema=qgis_test&project=abc"

        self.dropProjectsTable()  # make sure we have a clean start

        prj = QgsProject()
        uri = self.vl.source()
        vl1 = QgsVectorLayer(uri, 'test', 'postgres')
        self.assertEqual(vl1.isValid(), True)
        prj.addMapLayer(vl1)

        prj_storage = QgsApplication.projectStorageRegistry().projectStorageFromType("postgresql")
        self.assertTrue(prj_storage)

        lst0 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst0, [])

        # try to save project in the database

        prj.setFileName(project_uri)
        res = prj.write()
        self.assertTrue(res)

        lst1 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst1, ["abc"])

        # now try to load the project back

        prj2 = QgsProject()
        prj2.setFileName(project_uri)
        res = prj2.read()
        self.assertTrue(res)

        self.assertEqual(len(prj2.mapLayers()), 1)

        # try to see project's metadata

        res, metadata = prj_storage.readProjectMetadata(project_uri)
        self.assertTrue(res)
        time_project = metadata.lastModified
        time_now = QDateTime.currentDateTime()
        time_diff = time_now.secsTo(time_project)
        self.assertTrue(abs(time_diff) < 10)

        # try to remove the project

        res = prj_storage.removeProject(project_uri)
        self.assertTrue(res)

        lst2 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst2, [])

        self.dropProjectsTable()  # make sure we have a clean finish... "leave no trace"


if __name__ == '__main__':
    unittest.main()
