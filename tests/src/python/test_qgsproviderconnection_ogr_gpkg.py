# -*- coding: utf-8 -*-
"""QGIS Unit tests for OGR GeoPackage QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '10/08/2019'
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


class TestPyQgsProviderConnectionGpkg(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'ogr'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderConnectionBase.setUpClass()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_path = '{}/qgis_server/test_project_wms_grouped_layers_test.gpkg'.format(TEST_DATA_DIR)
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.gpkg_path)

    def test_gpkg_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(self.gpkg_path), 'test', 'ogr')
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        self.assertEqual(conn.uri(), self.gpkg_path)

    def test_gpkg_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.tableUri('', 'cdb_lines'), '{}|layername=cdb_lines'.format(self.gpkg_path))
        vl = QgsVectorLayer(conn.tableUri('', 'cdb_lines'), 'lines', 'ogr')
        self.assertTrue(vl.isValid())

        # Test table(), throws if not found
        table_info = conn.table('', 'osm')
        table_info = conn.table('', 'cdb_lines')

        self.assertEqual(conn.tableUri('', 'osm'), "GPKG:%s:osm" % self.uri)
        rl = QgsRasterLayer(conn.tableUri('', 'osm'), 'r', 'gdal')
        self.assertTrue(rl.isValid())

    def test_gpkg_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')

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

        # Check filters and special cases
        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertTrue('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.View))
        self.assertFalse('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertFalse('myNewAspatialTable' in table_names)

        table_names = self._table_names(conn.tables('', QgsAbstractDatabaseProviderConnection.Aspatial))
        self.assertFalse('osm' in table_names)
        self.assertFalse('myNewTable' in table_names)
        self.assertTrue('myNewAspatialTable' in table_names)


if __name__ == '__main__':
    unittest.main()
