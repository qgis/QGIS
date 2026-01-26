"""QGIS Unit tests for QgsConnectionRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os
import shutil
import tempfile

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsApplication,
    QgsProviderConnectionException,
    QgsProviderRegistry,
    QgsSettings,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# to find the srs.db
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsConnectionRegistry(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        QgsSettings().clear()

        gpkg_original_path = (
            f"{TEST_DATA_DIR}/qgis_server/test_project_wms_grouped_layers.gpkg"
        )
        cls.basetestpath = tempfile.mkdtemp()
        cls.gpkg_path = f"{cls.basetestpath}/test_gpkg.gpkg"
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer(f"{cls.gpkg_path}|layername=cdb_lines", "test", "ogr")
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.gpkg_path)
        super().tearDownClass()

    def testCreateConnectionBad(self):
        """
        Test creating connection with bad parameters
        """
        with self.assertRaises(QgsProviderConnectionException):
            QgsApplication.connectionRegistry().createConnection("invalid")

        with self.assertRaises(QgsProviderConnectionException):
            QgsApplication.connectionRegistry().createConnection("invalid://")

        with self.assertRaises(QgsProviderConnectionException):
            QgsApplication.connectionRegistry().createConnection("invalid://aa")

    def testCreateConnectionGood(self):
        # make a valid connection
        md = QgsProviderRegistry.instance().providerMetadata("ogr")
        conn = md.createConnection(self.gpkg_path, {})
        md.saveConnection(conn, "qgis_test1")

        conn = QgsApplication.connectionRegistry().createConnection("ogr://adasdas")
        self.assertFalse(conn.uri())

        conn = QgsApplication.connectionRegistry().createConnection("ogr://qgis_test1")
        self.assertEqual(conn.uri(), self.gpkg_path)

        # case insensitive provider name
        conn = QgsApplication.connectionRegistry().createConnection("OGR://qgis_test1")
        self.assertEqual(conn.uri(), self.gpkg_path)

        # connection name with spaces
        md.saveConnection(conn, "qgis Test 2")
        conn = QgsApplication.connectionRegistry().createConnection("OGR://qgis Test 2")
        self.assertEqual(conn.uri(), self.gpkg_path)


if __name__ == "__main__":
    unittest.main()
