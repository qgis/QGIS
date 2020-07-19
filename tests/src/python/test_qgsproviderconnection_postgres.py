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
    QgsDataSourceUri,
    QgsSettings,
)
from qgis.testing import unittest
from osgeo import gdal
from qgis.PyQt.QtCore import QTemporaryDir


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
                         '%s table="qgis_test"."Raster1"' % self.uri)

        rl = QgsRasterLayer(conn.tableUri('qgis_test', 'Raster1'), 'r1', 'postgresraster')
        self.assertTrue(rl.isValid())

    def test_sslmode_store(self):
        """Test that sslmode is stored as a string in the settings"""
        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection('database=\'mydb\' username=\'myuser\' password=\'mypasswd\' sslmode=verify-ca', {})
        conn.store('my_sslmode_test')
        settings = QgsSettings()
        settings.beginGroup('/PostgreSQL/connections/my_sslmode_test')
        self.assertEqual(settings.value("sslmode"), 'SslVerifyCa')
        self.assertEqual(settings.enumValue("sslmode", QgsDataSourceUri.SslPrefer), QgsDataSourceUri.SslVerifyCa)

    def test_postgis_geometry_filter(self):
        """Make sure the postgres provider only returns one matching geometry record and no polygons etc."""
        vl = QgsVectorLayer(self.postgres_conn + ' srid=4326 type=POINT table="qgis_test"."geometries_table" (geom) sql=', 'test', 'postgres')

        ids = [f.id() for f in vl.getFeatures()]
        self.assertEqual(ids, [2])

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

        # Check TopoGeometry and Pointcloud layers are found in vector table names
        tables = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Vector)
        table_names = self._table_names(tables)
        self.assertTrue('TopoLayer1' in table_names)
        self.assertTrue('PointCloudPointLayer' in table_names)
        self.assertTrue('PointCloudPatchLayer' in table_names)

        self.assertTrue('geometries_table' in table_names)

        # Revoke select permissions on topology.topology from qgis_test_user
        conn.executeSql('REVOKE SELECT ON topology.topology FROM qgis_test_user')

        # Revoke select permissions on pointcloud_format from qgis_test_user
        conn.executeSql('REVOKE SELECT ON pointcloud_formats FROM qgis_test_user')

        # Revoke select permissions on pointcloud_format from qgis_test_user
        conn.executeSql('REVOKE SELECT ON raster_columns FROM public')
        conn.executeSql('REVOKE SELECT ON raster_columns FROM qgis_test_user')

        # Re-connect as the qgis_test_role role
        newuri = self.uri + ' user=qgis_test_user password=qgis_test_user_password'
        newconn = md.createConnection(newuri, {})

        # Check TopoGeometry and Pointcloud layers are not found in vector table names
        tableTypes = QgsAbstractDatabaseProviderConnection.Vector | QgsAbstractDatabaseProviderConnection.Raster
        tables = newconn.tables('qgis_test', tableTypes)
        table_names = self._table_names(tables)
        self.assertFalse('TopoLayer1' in table_names)
        self.assertFalse('PointCloudPointLayer' in table_names)
        self.assertFalse('PointCloudPatchLayer' in table_names)
        self.assertFalse('Raster1' in table_names)
        self.assertTrue('geometries_table' in table_names)

        # TODO: only revoke select permission on topology.layer, grant
        #       on topology.topology

        # TODO: only revoke usage permission on topology, grant
        #       all on topology.layer and  topology.topology

        # TODO: only revoke select permission the actual topology
        #       schema associated with TopoLayer1

        # TODO: only revoke select permission the pointcloud_columns
        #       table

        # Grant select permissions back on topology.topology to qgis_test_user
        conn.executeSql('GRANT SELECT ON topology.topology TO qgis_test_user')

        # Grant select permissions back on pointcloud_formats to qgis_test_user
        conn.executeSql('GRANT SELECT ON pointcloud_formats TO qgis_test_user')

        # Grant select permissions back on raster_columns to qgis_test_user
        conn.executeSql('GRANT SELECT ON raster_columns TO public')
        conn.executeSql('GRANT SELECT ON raster_columns TO qgis_test_user')

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

    def test_true_false(self):
        """Test returned values from BOOL queries"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.executeSql('SELECT FALSE'), [[False]])
        self.assertEqual(conn.executeSql('SELECT TRUE'), [[True]])

    def test_nulls(self):
        """Test returned values from typed NULL queries"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.executeSql('SELECT NULL::bool'), [[None]])
        self.assertEqual(conn.executeSql('SELECT NULL::text'), [[None]])
        self.assertEqual(conn.executeSql('SELECT NULL::bytea'), [[None]])
        self.assertEqual(conn.executeSql('SELECT NULL::char'), [[None]])

    def test_pk_cols_order(self):
        """Test that PKs are returned in consistent order: see GH #34167"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.table('qgis_test', 'bikes_view').primaryKeyColumns(), ['pk', 'name'])
        self.assertEqual(conn.table('qgis_test', 'some_poly_data_view').primaryKeyColumns(), ['pk', 'geom'])

    def test_char_type_conversion(self):
        """Test char types: see GH #34806"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.executeSql("SELECT relname, relkind FROM pg_class c, pg_namespace n WHERE n.oid = c.relnamespace AND relname = 'bikes_view' AND c.relkind IN ('t', 'v', 'm')"), [['bikes_view', 'v']])

    def test_foreign_table_csv(self):
        """Test foreign table"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})
        temp_dir = QTemporaryDir()
        csv_path = os.path.join(temp_dir.path(), 'test.csv')
        csv = """id,description,geom_x,geom_y
1,Basic point,10.5,20.82
2,Integer point,11,22
3,Final point,13.0,23.0
"""
        with open(csv_path, 'w') as f:
            f.write(csv)

        os.chmod(temp_dir.path(), 0o777)
        os.chmod(csv_path, 0o777)

        foreign_table_definition = """
CREATE EXTENSION IF NOT EXISTS file_fdw;
CREATE SERVER IF NOT EXISTS file_fdw_test_server FOREIGN DATA WRAPPER file_fdw;
CREATE FOREIGN TABLE IF NOT EXISTS points_csv (
    id integer not null,
    name text,
    x numeric,
    y numeric ) SERVER file_fdw_test_server OPTIONS ( filename '%s', format 'csv', header 'true' );
""" % csv_path

        conn.executeSql(foreign_table_definition)

        self.assertNotEquals(conn.tables('public', QgsAbstractDatabaseProviderConnection.Foreign | QgsAbstractDatabaseProviderConnection.Aspatial), [])

    @unittest.skipIf(os.environ.get('TRAVIS', '') == 'true', 'Disabled on Travis')
    def test_foreign_table_server(self):
        """Test foreign table with server"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = md.createConnection(self.uri, {})

        uri = QgsDataSourceUri(conn.uri())
        host = uri.host()
        port = uri.port()
        user = uri.username()
        dbname = uri.database()
        password = uri.password()
        service = uri.service()

        foreign_table_definition = """
CREATE EXTENSION IF NOT EXISTS postgres_fdw;
CREATE SERVER IF NOT EXISTS postgres_fdw_test_server FOREIGN DATA WRAPPER postgres_fdw OPTIONS (service '{service}', dbname '{dbname}', host '{host}', port '{port}');
DROP SCHEMA  IF EXISTS foreign_schema CASCADE;
CREATE SCHEMA IF NOT EXISTS foreign_schema;
CREATE USER MAPPING IF NOT EXISTS FOR CURRENT_USER SERVER postgres_fdw_test_server OPTIONS (user '{user}', password '{password}');
IMPORT FOREIGN SCHEMA qgis_test LIMIT TO ( "someData" )
  FROM SERVER postgres_fdw_test_server
  INTO foreign_schema;
""".format(host=host, user=user, port=port, dbname=dbname, password=password, service=service)
        conn.executeSql(foreign_table_definition)
        self.assertEquals(conn.tables('foreign_schema', QgsAbstractDatabaseProviderConnection.Foreign)[0].tableName(), 'someData')

    def test_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('qgis_test', 'someData')
        self.assertEqual(fields.names(), ['pk', 'cnt', 'name', 'name2', 'num_char', 'dt', 'date', 'time', 'geom'])


if __name__ == '__main__':
    unittest.main()
