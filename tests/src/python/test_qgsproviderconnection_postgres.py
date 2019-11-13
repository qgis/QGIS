# -*- coding: utf-8 -*-
"""QGIS Unit tests for Postgres QgsAbastractProviderConnection API.

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
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.core import (
    QgsWkbTypes,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
    QgsVectorLayer,
    QgsProviderRegistry,
    QgsCoordinateReferenceSystem,
    QgsRasterLayer,
)
from qgis.testing import unittest
from osgeo import gdal


class TestPyQgsProviderConnectionPostgres(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'postgres'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        TestPyQgsProviderConnectionBase.setUpClass()
        cls.postgres_conn = "service='qgis_test'"
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.postgres_conn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        vl = QgsVectorLayer(cls.postgres_conn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert vl.isValid()
        cls.uri = cls.postgres_conn + ' sslmode=disable'

    def test_postgis_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        vl = QgsVectorLayer(self.postgres_conn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        self.assertEqual(conn.uri(), self.uri)

        # Test table(), throws if not found
        table_info = conn.table('qgis_test', 'someData')
        table_info = conn.table('qgis_test', 'Raster1')

        # Test raster
        self.assertEqual(conn.tableUri('qgis_test', 'Raster1'),
                         'PG: %s mode=2 schema=\'qgis_test\' table=\'Raster1\' column=\'Rast\'' % self.uri)

        if (gdal.VersionInfo() >= '2040000'):
            rl = QgsRasterLayer(conn.tableUri('qgis_test', 'Raster1'), 'r1', 'gdal')
            self.assertTrue(rl.isValid())

    def test_postgis_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        vl = QgsVectorLayer(conn.tableUri('qgis_test', 'geometries_table'), 'my', 'postgres')
        self.assertTrue(vl.isValid())

    def test_postgis_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameRasterTable))

        # Check filters and special cases
        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertTrue('Raster1' in table_names)
        self.assertFalse('geometryless_table' in table_names)
        self.assertFalse('geometries_table' in table_names)
        self.assertFalse('geometries_view' in table_names)

        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.View))
        self.assertFalse('Raster1' in table_names)
        self.assertFalse('geometryless_table' in table_names)
        self.assertFalse('geometries_table' in table_names)
        self.assertTrue('geometries_view' in table_names)

        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Aspatial))
        self.assertFalse('Raster1' in table_names)
        self.assertTrue('geometryless_table' in table_names)
        self.assertFalse('geometries_table' in table_names)
        self.assertFalse('geometries_view' in table_names)

        tables = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Aspatial | QgsAbstractDatabaseProviderConnection.View)
        table_names = self._table_names(tables)
        b32523_view = self._table_by_name(tables, 'b32523')
        self.assertTrue(b32523_view)
        pks = b32523_view.primaryKeyColumns()
        self.assertTrue('pk' in pks)
        self.assertTrue('random' in pks)

        geometries_table = self._table_by_name(conn.tables('qgis_test'), 'geometries_table')
        srids_and_types = [[t.crs.postgisSrid(), t.wkbType]
                           for t in geometries_table.geometryColumnTypes()]
        srids_and_types.sort()
        self.assertEqual(srids_and_types,
                         [[0, 1], [0, 2], [0, 3], [0, 7], [3857, 1], [4326, 1]])

        # Check TopoGeometry layers are found in vector table names

        tables = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Vector)
        table_names = self._table_names(tables)
        self.assertTrue('TopoLayer1' in table_names)
        self.assertTrue('geometries_table' in table_names)

        # Revoke select permissions on topology.topology from qgis_test_user
        conn.executeSql('REVOKE SELECT ON topology.topology FROM qgis_test_user')

        # Re-connect as the qgis_test_role role
        newuri = self.uri + ' user=qgis_test_user password=qgis_test_user_password'
        conn = md.createConnection(newuri, {})

        tables = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Vector)
        table_names = self._table_names(tables)
        self.assertFalse('TopoLayer1' in table_names)
        self.assertTrue('geometries_table' in table_names)

        # TODO: only revoke select permission on topology.layer, grant
        #       on topology.topology

        # TODO: only revoke usage permission on topology, grant
        #       all on topology.layer and  topology.topology

        # TODO: only revoke select permission the actual topology
        #       schema associated with TopoLayer1

    # error: ERROR: relation "qgis_test.raster1" does not exist
    @unittest.skipIf(gdal.VersionInfo() < '2040000', 'This test requires GDAL >= 2.4.0')
    def test_postgis_raster_rename(self):
        """Test raster rename"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        table = self._table_by_name(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster), 'Raster1')
        self.assertTrue(QgsRasterLayer("PG: %s schema='qgis_test' column='%s' table='%s'" % (conn.uri(), table.geometryColumn(), table.tableName()), 'r1', 'gdal').isValid())
        conn.renameRasterTable('qgis_test', table.tableName(), 'Raster2')
        table = self._table_by_name(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster), 'Raster2')
        self.assertTrue(QgsRasterLayer("PG: %s schema='qgis_test' column='%s' table='%s'" % (conn.uri(), table.geometryColumn(), table.tableName()), 'r1', 'gdal').isValid())
        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertFalse('Raster1' in table_names)
        self.assertTrue('Raster2' in table_names)
        conn.renameRasterTable('qgis_test', table.tableName(), 'Raster1')
        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertFalse('Raster2' in table_names)
        self.assertTrue('Raster1' in table_names)


if __name__ == '__main__':
    unittest.main()
