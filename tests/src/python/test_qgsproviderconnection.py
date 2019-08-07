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
        sl_original_path = '{}/provider/spatialite.db'.format(TEST_DATA_DIR)
        cls.spatialite_path = '{}/provider/spatialite_test.db'.format(TEST_DATA_DIR)
        shutil.copy(sl_original_path, cls.spatialite_path)
        vl = QgsVectorLayer('dbname=\'{}\' table="somedata" (geom) sql='.format(cls.spatialite_path), 'test', 'spatialite')
        assert vl.isValid()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_path = '{}/qgis_server/test_project_wms_grouped_layers_test.gpkg'.format(TEST_DATA_DIR)
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        os.unlink(cls.spatialite_path)
        os.unlink(cls.gpkg_path)

    def setUp(self):
        QgsSettings().clear()

    def _test_save_load(self, md, uri):
        """Common tests on connection save and load"""
        conn = md.connection('qgis_test1', uri)
        md.saveConnection(conn)
        conn = list(md.connections().values())[0]
        self.assertEqual(conn.name(), 'qgis_test1')
        return conn

    def _test_operations(self, md, conn):
        """Common tests"""

        capabilities = conn.capabilities()

        # Schema operations
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema
                and capabilities & QgsAbstractDatabaseProviderConnection.Schemas
                and capabilities & QgsAbstractDatabaseProviderConnection.RenameSchema
                and capabilities & QgsAbstractDatabaseProviderConnection.DropSchema ):
            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema and 'myNewSchema' in conn.schemas():
                conn.dropSchema('myNewSchema')
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
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable
                and capabilities & QgsAbstractDatabaseProviderConnection.Tables
                and capabilities & QgsAbstractDatabaseProviderConnection.RenameTable
                and capabilities & QgsAbstractDatabaseProviderConnection.DropTable ):

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema and 'myNewSchema' in conn.schemas():
                conn.dropSchema('myNewSchema')
            if capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema:
                schema = 'myNewSchema'
                conn.createSchema('myNewSchema')
            else:
                schema = 'public'

            if 'myNewTable' in conn.tables('qgis_test')[0]:
                conn.dropTable(schema, 'myNewTable')
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
            tables = conn.tables(schema)
            self.assertTrue('myNewTable' in tables)
            # Rename
            conn.renameTable(schema, 'myNewTable', 'myVeryNewTable')
            tables = conn.tables(schema)
            self.assertFalse('myNewTable' in tables)
            self.assertTrue('myVeryNewTable' in tables)
            # Vacuum
            if capabilities & QgsAbstractDatabaseProviderConnection.Vacuum:
                conn.vacuum('myNewSchema', 'myVeryNewTable')
            # Drop
            conn.dropTable(schema, 'myVeryNewTable')
            tables = conn.tables(schema)
            self.assertFalse('myVeryNewTable' in tables)

        conns = md.connections()
        self.assertTrue(isinstance(list(conns.values())[0], QgsAbstractDatabaseProviderConnection))

        # Remove connection
        md.deleteConnection(conn.name())
        self.assertEqual(list(md.connections().values()), [])

    def ___test_postgis_connections(self):
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
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateRasterTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameTable))

        # Run common tests
        self._test_operations(md, conn)

    def ______test_spatialite_connections(self):
        """Test spatialite connections"""

        md = QgsProviderRegistry.instance().providerMetadata('spatialite')
        uri = 'dbname=\'{}\' table="somedata" (geom) sql='.format(self.spatialite_path)
        conn = md.connection('qgis_test1', uri)
        md.saveConnection(conn)

    def test_geopackage_connections(self):
        """Test geopackage connections"""

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        uri = self.gpkg_path

        # Run common tests
        conn = self._test_save_load(md, uri)
        # Check that the raster is in the tables
        self.assertTrue('osm' in conn.tables())
        # Run common tests
        self._test_operations(md, conn)

    @unittest.skip('Still have to find a way to get QgsProviderConnectionException instead of "unknown"')
    def test_errors(self):
        """Test SQL errors"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        uri = self.postgres_conn + ' port=5432 sslmode=disable key=\'gid\''
        conn = md.connection('qgis_test1', uri)
        md.saveConnection(conn)

        with self.assertRaises(QgsProviderConnectionException) as ex:
            conn.createRasterTable('notExists', 'notReally')

        with self.assertRaises(QgsProviderConnectionException) as ex:
            conn.executeSql('DROP TABLE "notExists"')

        # Remove connection
        md.deleteConnection(conn.name())
        self.assertEqual(list(md.connections().values()), [])


if __name__ == '__main__':
    unittest.main()
