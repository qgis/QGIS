# -*- coding: utf-8 -*-
"""QGIS Unit tests for Spatialite QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '28/10/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.core import (
    QgsWkbTypes,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProviderRegistry,
    QgsFields,
    QgsCoordinateReferenceSystem,
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnectionSpatialite(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'spatialite'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderConnectionBase.setUpClass()
        spatialite_original_path = '{}/qgis_server/test_project_wms_grouped_layers.sqlite'.format(TEST_DATA_DIR)
        cls.spatialite_path = '{}/qgis_server/test_project_wms_grouped_layers_test.sqlite'.format(TEST_DATA_DIR)
        shutil.copy(spatialite_original_path, cls.spatialite_path)
        cls.uri = "dbname=\'%s\'" % cls.spatialite_path
        vl = QgsVectorLayer('{} table=\'cdb_lines\''.format(cls.uri), 'test', 'spatialite')
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.spatialite_path)

    def test_spatialite_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        vl = QgsVectorLayer('{} table=\'cdb_lines\''.format(self.uri), 'test', 'spatialite')
        self.assertTrue(vl.isValid())
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        self.assertEqual(conn.uri(), self.uri + ' table="cdb_lines"')
        conn.tables()

    def test_spatialite_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.tableUri('', 'cdb_lines'), '{} table="cdb_lines"'.format(self.uri))
        vl = QgsVectorLayer(conn.tableUri('', 'cdb_lines'), 'lines', 'spatialite')
        self.assertTrue(vl.isValid())

        # Test table(), throws if not found
        table_info = conn.table('', 'cdb_lines')

    def test_spatialite_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable))
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameRasterTable))

        crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
        typ = QgsWkbTypes.LineString
        conn.createVectorTable('', 'myNewAspatialTable', QgsFields(), QgsWkbTypes.NoGeometry, crs, True, {})
        conn.createVectorTable('', 'myNewTable', QgsFields(), typ, crs, True, {})

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.View))
        self.assertTrue('my_view' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Aspatial))
        self.assertFalse('myNewTable' in table_names)
        self.assertTrue('myNewAspatialTable' in table_names)


if __name__ == '__main__':
    unittest.main()
