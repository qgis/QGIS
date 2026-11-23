"""QGIS Unit tests for the postgres project storage.

Note: to prepare the DB, you need to run the sql files specified in
tests/testdata/provider/testdata_pg.sh

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Martin Dobias"
__date__ = "2018-03-29"
__copyright__ = "Copyright 2018, The QGIS Project"

import os
import unittest

import psycopg2
from qgis.core import (
    QgsApplication,
    QgsDataSourceUri,
    QgsProject,
    QgsProjectItem,
    QgsProviderRegistry,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QUrl, QUrlQuery
from qgis.testing import QgisTestCase, start_app
from test_project_storage_base import TestPyQgsProjectStorageBase
from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProjectStoragePostgres(QgisTestCase, TestPyQgsProjectStorageBase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        super().setUpClass()

        cls.dbconn = "service=qgis_test"
        if "QGIS_PGTEST_DB" in os.environ:
            cls.dbconn = os.environ["QGIS_PGTEST_DB"]
        cls.ds_uri = QgsDataSourceUri(cls.dbconn)

        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn
            + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            "test",
            "postgres",
        )
        assert cls.vl.isValid()
        cls.con = psycopg2.connect(cls.dbconn)

        cls.schema = "qgis_test"
        cls.provider = "postgres"
        cls.project_storage_type = "postgresql"

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
        if ds_uri.port() != "":
            u.setPort(int(ds_uri.port()))
        if ds_uri.username() != "":
            u.setUserName(ds_uri.username())
        if ds_uri.password() != "":
            u.setPassword(ds_uri.password())

        if ds_uri.service() != "":
            urlQuery.addQueryItem("service", ds_uri.service())
        if ds_uri.authConfigId() != "":
            urlQuery.addQueryItem("authcfg", ds_uri.authConfigId())
        if ds_uri.sslMode() != QgsDataSourceUri.SslMode.SslPrefer:
            urlQuery.addQueryItem(
                "sslmode", QgsDataSourceUri.encodeSslMode(ds_uri.sslMode())
            )

        urlQuery.addQueryItem("dbname", ds_uri.database())

        urlQuery.addQueryItem("schema", schema_name)
        if project_name:
            urlQuery.addQueryItem("project", project_name)

        u.setQuery(urlQuery)
        return str(u.toEncoded(), "utf-8")

    def test_browser_project_item(self):
        """Test project items and their tooltips in the browser"""

        # helper to find item in list by name
        def find_item(items: list[QgsProjectItem], name: str):
            return next((item for item in items if item.name() == name), None)

        # helper to find project item by name in the Postgres browser tree
        def find_project_item_in_postgres(
            project_name: str,
        ) -> tuple[
            QgsProjectItem, tuple[QgsProjectItem, QgsProjectItem, QgsProjectItem]
        ]:
            root_item = (
                QgsApplication.dataItemProviderRegistry()
                .provider("PostGIS")
                .createDataItem("", None)
            )
            conn_item = find_item(root_item.createChildren(), "browser_projects_test")
            self.assertIsNotNone(conn_item)
            schema_item = find_item(conn_item.createChildren(), self.schema)
            self.assertIsNotNone(schema_item)
            project_item = find_item(schema_item.createChildren(), project_name)
            self.assertIsInstance(project_item, QgsProjectItem)
            # return all objects to avoid deletion of parents
            return (project_item, (root_item, conn_item, schema_item))

        self.dropProjectsTable()

        # provider metadata and connection
        md = QgsProviderRegistry.instance().providerMetadata(self.provider)
        conn = md.createConnection(self.dbconn, {"projectsInDatabase": True})
        md.saveConnection(conn, "browser_projects_test")

        project_name = "pg_project_for_testing_browser"
        comment_str = "test comment"

        # project with vector layer saved in DB
        project = QgsProject()
        project.addMapLayer(QgsVectorLayer(self.vl.source(), "test", self.provider))
        project.setFileName(self.encode_uri(self.ds_uri, self.schema, project_name))
        self.assertTrue(project.write())

        # add comment to project in DB
        self.execSQLCommand(
            f"UPDATE qgis_test.qgis_projects SET comment = '{comment_str}' WHERE name = '{project_name}';"
        )

        # test creation of project item in Postgres browser tree and  tooltip
        project_item, _parents = find_project_item_in_postgres(project_name)
        self.assertIsInstance(project_item, QgsProjectItem)
        tooltip_lines = project_item.toolTip().split("\n")
        self.assertEqual(len(tooltip_lines), 3)
        self.assertTrue(tooltip_lines[0].startswith("Last modified time: "))
        self.assertTrue(tooltip_lines[1].startswith("Last modified user: "))
        self.assertEqual(tooltip_lines[2], f"Comment: {comment_str}")

        # historic tables might not have comment column, test that it works without it
        self.execSQLCommand("ALTER TABLE qgis_test.qgis_projects DROP COLUMN comment;")

        project_item, _parents = find_project_item_in_postgres(project_name)
        self.assertIsInstance(project_item, QgsProjectItem)
        tooltip_lines = project_item.toolTip().split("\n")
        self.assertEqual(len(tooltip_lines), 3)
        self.assertTrue(tooltip_lines[0].startswith("Last modified time: "))
        self.assertTrue(tooltip_lines[1].startswith("Last modified user: "))
        self.assertEqual(tooltip_lines[2], "Comment: ")

        # cleanup
        self.dropProjectsTable()
        md.deleteConnection("browser_projects_test")


if __name__ == "__main__":
    unittest.main()
