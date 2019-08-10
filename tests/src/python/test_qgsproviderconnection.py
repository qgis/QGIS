# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '05/08/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.testing import start_app, unittest
from qgis.core import (
    QgsSettings,
    QgsProviderRegistry,
    QgsWkbTypes,
    QgsVectorLayer,
    QgsFields,
    QgsCoordinateReferenceSystem,
    QgsField,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
)

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsProviderConnection(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()
        cls.postgres_conn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.postgres_conn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        vl = QgsVectorLayer(cls.postgres_conn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert vl.isValid()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_path = '{}/qgis_server/test_project_wms_grouped_layers_test.gpkg'.format(TEST_DATA_DIR)
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.gpkg_path)

    def setUp(self):
        QgsSettings().clear()

    def _test_save_load(self, md, uri):
        """Common tests on connection save and load"""
        conn = md.connection('qgis_test1', uri)
        md.saveConnection(conn)
        return md.connections()['qgis_test1']

    def _table_names(self, table_properties):
        """Return table default names from table property list"""
        return [p.defaultName() for p in table_properties]

    def _table_by_name(self, table_properties, name):
        """Filter table properties by table name"""
        try:
            return [p for p in table_properties if p.defaultName() == name][0]
        except IndexError:
            return None

    def _test_operations(self, md, conn):
        """Common tests"""

        capabilities = conn.capabilities()

        # Schema operations
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema and
                capabilities & QgsAbstractDatabaseProviderConnection.Schemas and
                capabilities & QgsAbstractDatabaseProviderConnection.RenameSchema and
                capabilities & QgsAbstractDatabaseProviderConnection.DropSchema ):
            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema and 'myNewSchema' in conn.schemas():
                conn.dropSchema('myNewSchema', True)
            # Create
            conn.createSchema('myNewSchema')
            schemas = conn.schemas()
            self.assertTrue('myNewSchema' in schemas)
            # Rename
            conn.renameSchema('myNewSchema', 'myVeryNewSchema')
            schemas = conn.schemas()
            self.assertTrue('myVeryNewSchema' in schemas)
            self.assertFalse('myNewSchema' in schemas)
            # Drop
            conn.dropSchema('myVeryNewSchema')
            schemas = conn.schemas()
            self.assertFalse('myVeryNewSchema' in schemas)

        # Table operations
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable and
                capabilities & QgsAbstractDatabaseProviderConnection.Tables and
                capabilities & QgsAbstractDatabaseProviderConnection.RenameTable and
                capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable ):

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema and 'myNewSchema' in conn.schemas():
                conn.dropSchema('myNewSchema', True)
            if capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema:
                schema = 'myNewSchema'
                conn.createSchema('myNewSchema')
            else:
                schema = 'public'

            if 'myNewTable' in self._table_names(conn.tables(schema)):
                conn.dropVectorTable(schema, 'myNewTable')
            fields = QgsFields()
            fields.append(QgsField("string", QVariant.String))
            fields.append(QgsField("long", QVariant.LongLong))
            fields.append(QgsField("double", QVariant.Double))
            fields.append(QgsField("integer", QVariant.Int))
            fields.append(QgsField("date", QVariant.Date))
            fields.append(QgsField("datetime", QVariant.DateTime))
            fields.append(QgsField("time", QVariant.Time))
            options = {}
            crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
            typ = QgsWkbTypes.LineString
            # Create
            conn.createVectorTable(schema, 'myNewTable', fields, typ, crs, True, options)
            table_names = self._table_names(conn.tables(schema))
            self.assertTrue('myNewTable' in table_names)

            # Check table information
            table_properties = conn.tables(schema)
            table_property = self._table_by_name(table_properties, 'myNewTable')
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.tableName(), 'myNewTable')
            self.assertEqual(table_property.geometryColumnCount(), 1)
            self.assertEqual(table_property.geometryTypes()[0], (QgsWkbTypes.LineString, 3857))
            self.assertEqual(table_property.defaultName(), 'myNewTable')

            # Check aspatial tables
            conn.createVectorTable(schema, 'myNewAspatialTable', fields, QgsWkbTypes.NoGeometry, crs, True, options)
            table_properties = conn.tables(schema, QgsAbstractDatabaseProviderConnection.Aspatial)
            table_property = self._table_by_name(table_properties, 'myNewAspatialTable')
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.tableName(), 'myNewAspatialTable')
            self.assertEqual(table_property.geometryColumnCount(), 0)
            self.assertEqual(table_property.geometryColumn(), '')
            self.assertEqual(table_property.defaultName(), 'myNewAspatialTable')
            self.assertEqual(table_property.geometryTypes()[0][0], QgsWkbTypes.NoGeometry)
            self.assertTrue(table_property.geometryTypes()[0][1] <= 0)
            self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Raster)
            self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Vector)
            self.assertTrue(table_property.flags() & QgsAbstractDatabaseProviderConnection.Aspatial)

            # Check executeSql
            has_schema = capabilities & QgsAbstractDatabaseProviderConnection.Schemas
            if capabilities & QgsAbstractDatabaseProviderConnection.ExecuteSql:
                if has_schema:
                    table = "\"%s\".\"myNewAspatialTable\"" % schema
                else:
                    table = 'myNewAspatialTable'
                sql = "INSERT INTO %s (string, integer) VALUES ('QGIS Rocks - \U0001f604', 666)" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = "SELECT string, integer FROM %s" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [['QGIS Rocks - \U0001f604', '666']])
                sql = "DELETE FROM %s WHERE string = 'QGIS Rocks - \U0001f604'" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = "SELECT string, integer FROM %s" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [])

            # Check that we do NOT get the aspatial table when querying for vectors
            table_names = self._table_names(conn.tables(schema, QgsAbstractDatabaseProviderConnection.Vector))
            self.assertTrue('myNewTable' in table_names)
            self.assertFalse('myNewAspatialTable' in table_names)

            # Query for rasters (in qgis_test schema or no schema for GPKG)
            table_properties = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster)
            # At leasy one raster should be there
            self.assertTrue(len(table_properties) >= 1)
            table_property = table_properties[0]
            self.assertTrue(table_property.flags() & QgsAbstractDatabaseProviderConnection.Raster)
            self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Vector)
            self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Aspatial)

            # Rename
            conn.renameTable(schema, 'myNewTable', 'myVeryNewTable')
            tables = self._table_names(conn.tables(schema))
            self.assertFalse('myNewTable' in tables)
            self.assertTrue('myVeryNewTable' in tables)
            # Vacuum
            if capabilities & QgsAbstractDatabaseProviderConnection.Vacuum:
                conn.vacuum('myNewSchema', 'myVeryNewTable')

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema:
                # Drop schema (should fail)
                with self.assertRaises(QgsProviderConnectionException) as ex:
                    conn.dropSchema('myNewSchema')

            # Drop table
            conn.dropVectorTable(schema, 'myVeryNewTable')
            conn.dropVectorTable(schema, 'myNewAspatialTable')
            table_names = self._table_names(conn.tables(schema))
            self.assertFalse('myVeryNewTable' in table_names)

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema:
                # Drop schema
                conn.dropSchema('myNewSchema')
                self.assertFalse('myNewSchema' in conn.schemas())

        conns = md.connections()
        self.assertTrue(isinstance(list(conns.values())[0], QgsAbstractDatabaseProviderConnection))

        # Remove connection
        md.deleteConnection(conn.name())
        self.assertEqual(list(md.connections().values()), [])

    def test_postgis_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        uri = self.postgres_conn + ' port=5432 sslmode=disable key=\'gid\''

        # Run common tests
        conn = self._test_save_load(md, uri)

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameTable))

        # Run common tests
        self._test_operations(md, conn)

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

        geometries_table = self._table_by_name(conn.tables('qgis_test'), 'geometries_table')
        self.assertEqual(geometries_table.geometryTypes(), [(7, 0), (2, 0), (1, 0), (1, 3857), (1, 4326), (3, 0)])

    def test_geopackage_connections(self):
        """Test geopackage connections"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        uri = self.gpkg_path

        # Run common tests
        conn = self._test_save_load(md, uri)
        # Check that the raster is in the tables
        self.assertTrue('osm' in self._table_names(conn.tables()))
        # Run common tests
        self._test_operations(md, conn)

    def test_errors(self):
        """Test SQL errors"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        uri = self.postgres_conn + ' port=5432 sslmode=disable key=\'gid\''
        conn = md.connection('qgis_test1', uri)
        md.saveConnection(conn)

        with self.assertRaises(QgsProviderConnectionException) as ex:
            conn.createVectorTable('notExists', 'notReally', QgsFields(), QgsWkbTypes.Point, QgsCoordinateReferenceSystem(), False, {})

        with self.assertRaises(QgsProviderConnectionException) as ex:
            conn.executeSql('DROP TABLE "notExists"')

        # Remove connection
        md.deleteConnection(conn.name())
        self.assertEqual(list(md.connections().values()), [])


if __name__ == '__main__':
    unittest.main()
