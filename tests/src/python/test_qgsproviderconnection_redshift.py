# -*- coding: utf-8 -*-
"""QGIS Unit tests for Redshift QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Marcel Bezdrighin'
__date__ = '18/02/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from qgis.PyQt.QtCore import QDir, QVariant, QTemporaryDir
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
    QgsField,
    QgsFields
)
from qgis.testing import unittest
from osgeo import gdal
from qgis.PyQt import QtCore
from qgis.PyQt.QtTest import QSignalSpy


class TestPyQgsProviderConnectionRedshift(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'redshift'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        TestPyQgsProviderConnectionBase.setUpClass()
        cls.redshift_conn = os.environ['QGIS_RSTEST_DB']

        # Create test layers
        vl = QgsVectorLayer(cls.redshift_conn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."some_data" (geom) sql=', 'test', 'redshift')

        assert vl.isValid()
        cls.uri = cls.redshift_conn + ' sslmode=disable'

    def test_redshift_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('redshift')
        vl = QgsVectorLayer(self.redshift_conn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."some_data" (geom) sql=', 'test', 'redshift')
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        # self.assertEqual(conn.uri(), self.uri)

        # Test table(), throws if not found
        table_info = conn.table('qgis_test', 'some_data')

    def test_sslmode_store(self):
        """Test that sslmode is stored as a string in the settings"""
        md = QgsProviderRegistry.instance().providerMetadata('redshift')
        conn = md.createConnection('database=\'mydb\' username=\'myuser\' password=\'mypasswd\' sslmode=verify-ca', {})
        conn.store('my_sslmode_test')
        settings = QgsSettings()
        settings.beginGroup('/Redshift/connections/my_sslmode_test')
        self.assertEqual(settings.value("sslmode"), 'SslVerifyCa')
        self.assertEqual(settings.enumValue("sslmode", QgsDataSourceUri.SslPrefer), QgsDataSourceUri.SslVerifyCa)

    def test_redshift_geometry_filter(self):
        """Make sure the redshift provider only returns one matching geometry record and no polygons etc."""
        vl = QgsVectorLayer(self.redshift_conn + ' srid=4326 type=POINT table="qgis_test"."geometries_table" (geom) sql=', 'test', 'redshift')
        assert vl.isValid()

        names = [f.attribute('name') for f in vl.getFeatures()]
        self.assertEqual(names, ['Point4326'])

        # don't restrict SRID, should fail because can't determine unique srid
        vl = QgsVectorLayer(self.redshift_conn + ' type=POINT table="qgis_test"."geometries_table" (geom) sql=', 'test', 'redshift')
        assert not vl.isValid()

    def test_redshift_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('redshift')
        conn = md.createConnection(self.uri, {})
        vl = QgsVectorLayer(conn.tableUri('qgis_test', 'geometries_table'), 'my', 'redshift')
        self.assertTrue(vl.isValid())

    def test_redshift_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('redshift')

        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable))

        # Check filters and special cases
        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster))
        self.assertFalse('geometryless_table' in table_names)
        self.assertFalse('geometries_table' in table_names)
        self.assertFalse('geometries_view' in table_names)

        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.View))
        self.assertFalse('geometryless_table' in table_names)
        self.assertFalse('geometries_table' in table_names)
        self.assertTrue('geometries_view' in table_names)

        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Aspatial))
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
                         [
                             # [0, 1], [0, 2], [0, 3], [0, 7],
                             [3857, 1], [4326, 1]])

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
        self.assertEqual(conn.executeSql('SELECT NULL::time'), [[None]])
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

    def test_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('redshift')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('qgis_test', 'some_data')
        self.assertEqual(fields.names(), ['pk', 'cnt', 'name', 'name2', 'num_char', 'dt', 'date', 'time', 'geom'])

    # Override _test_operations in TestPyQgsProviderConnectionBase because Redshift is case insensitive
    def _test_operations(self, md, conn):
        """Common tests"""

        capabilities = conn.capabilities()

        # Schema operations
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema
            and capabilities & QgsAbstractDatabaseProviderConnection.Schemas
                and capabilities & QgsAbstractDatabaseProviderConnection.DropSchema):

            # Start clean
            if 'mynewschema' in conn.schemas():
                conn.dropSchema('myNewSchema', True)

            # Create
            conn.createSchema('myNewSchema')
            schemas = conn.schemas()
            self.assertTrue('mynewschema' in schemas)  # Redshift uses lower case for identifiers

            # Create again
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.createSchema('myNewScHeMa')

            # Test rename
            if capabilities & QgsAbstractDatabaseProviderConnection.RenameSchema:
                # Rename
                conn.renameSchema('myNewSchema', 'myVeryNewSchema')
                schemas = conn.schemas()
                self.assertTrue('myverynewschema' in schemas)
                self.assertFalse('mynewschema' in schemas)
                conn.renameSchema('myveRynewsChema', 'myNewScheMA')
                schemas = conn.schemas()
                self.assertFalse('myverynewschema' in schemas)
                self.assertTrue('mynewschema' in schemas)

            # Drop
            conn.dropSchema('myNewSchema')
            schemas = conn.schemas()
            self.assertFalse('mynewschema' in schemas)

            # UTF8 schema
            conn.createSchema('myutf8\U0001f604newScHema')
            schemas = conn.schemas()
            self.assertTrue('myutf8\U0001f604newschema' in schemas)
            conn.dropSchema('myutf8\U0001f604newsChema')
            schemas = conn.schemas()
            self.assertFalse('myutf8\U0001f604newschema' in schemas)

        # Table operations
        if (capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable
            and capabilities & QgsAbstractDatabaseProviderConnection.Tables
                and capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable):

            if capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema:
                schema = 'myNewSchema'
                conn.createSchema('myNewSchema')
            else:
                schema = 'public'

            # Start clean
            if 'mynewtable' in self._table_names(conn.tables(schema)):
                conn.dropVectorTable(schema, 'myNewTable')

            fields = QgsFields()
            fields.append(QgsField("string_t", QVariant.String))
            fields.append(QgsField("long_t", QVariant.LongLong))
            fields.append(QgsField("double_t", QVariant.Double))
            fields.append(QgsField("integer_t", QVariant.Int))
            fields.append(QgsField("date_t", QVariant.Date))
            fields.append(QgsField("datetime_t", QVariant.DateTime))
            fields.append(QgsField("time_t", QVariant.Time))
            options = {}
            crs = QgsCoordinateReferenceSystem.fromEpsgId(3857)
            typ = QgsWkbTypes.LineString

            # Create
            conn.createVectorTable(schema, 'myNewTable', fields, typ, crs, True, options)
            table_names = self._table_names(conn.tables(schema))
            self.assertTrue('mynewtable' in table_names)

            # Create UTF8 table
            conn.createVectorTable(schema, 'myUtf8\U000000EDTable', fields, typ, crs, True, options)
            table_names = self._table_names(conn.tables(schema))
            self.assertTrue('mynewtable' in table_names)
            self.assertTrue('myutf8\U000000EDtable' in table_names)
            conn.dropVectorTable(schema, 'myutf8\U000000EDTable')
            table_names = self._table_names(conn.tables(schema))
            self.assertFalse('myutf8\U000000EDtable' in table_names)
            self.assertTrue('mynewtable' in table_names)

            # Check table information
            table_properties = conn.tables(schema)
            table_property = self._table_by_name(table_properties, 'mynewtable')
            self.assertEqual(table_property.maxCoordinateDimensions(), 2)
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.tableName(), 'mynewtable')  # Redshift lowercases names
            self.assertEqual(table_property.geometryColumnCount(), 1)
            self.assertEqual(table_property.geometryColumnTypes()[0].wkbType, QgsWkbTypes.LineString)
            cols = table_property.geometryColumnTypes()
            self.assertEqual(cols[0].crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(table_property.defaultName(), 'mynewtable')

            # Check aspatial tables
            conn.createVectorTable(schema, 'myNewAspatialTable', fields, QgsWkbTypes.NoGeometry, crs, True, options)
            table_properties = conn.tables(schema, QgsAbstractDatabaseProviderConnection.Aspatial)
            table_property = self._table_by_name(table_properties, 'mynewaspatialtable')
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.maxCoordinateDimensions(), 0)
            self.assertEqual(table_property.tableName(), 'mynewaspatialtable')
            self.assertEqual(table_property.geometryColumnCount(), 0)
            self.assertEqual(table_property.geometryColumn(), '')
            self.assertEqual(table_property.defaultName(), 'mynewaspatialtable')
            cols = table_property.geometryColumnTypes()
            # We always return geom col types, even when there is no geometry
            self.assertEqual(cols[0].wkbType, QgsWkbTypes.NoGeometry)
            self.assertFalse(cols[0].crs.isValid())
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

                sql = "INSERT INTO %s (string_t, long_t, double_t, integer_t, date_t, datetime_t, time_t) VALUES ('QGIS Rocks - \U0001f604', 666, 1.234, 1234, '2019-07-08', '2019-07-08T12:00:12', '12:00:13.00')" % (
                    table)
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = "SELECT string_t, long_t, double_t, integer_t, date_t, datetime_t FROM %s" % table
                res = conn.executeSql(sql)

                self.assertEqual(res, [['QGIS Rocks - \U0001f604', 666, 1.234, 1234, QtCore.QDate(2019, 7, 8),
                                        QtCore.QDateTime(2019, 7, 8, 12, 0, 12)]])
                sql = "SELECT time_t FROM %s" % table
                res = conn.executeSql(sql)

                self.assertIn(res, ([[QtCore.QTime(12, 0, 13)]], [['12:00:13.00']]))

                sql = "DELETE FROM %s WHERE string_t = 'QGIS Rocks - \U0001f604'" % (
                    table)
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = "SELECT string_t, integer_t FROM %s" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [])

            # Check that we do NOT get the aspatial table when querying for vectors
            table_names = self._table_names(conn.tables(schema, QgsAbstractDatabaseProviderConnection.Vector))
            self.assertTrue('mynewtable' in table_names)
            self.assertFalse('mynewaspatialtable' in table_names)

            if capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable:
                # Rename
                conn.renameVectorTable(schema, 'myNewTable', 'myVeryNewTabLe')
                tables = self._table_names(conn.tables(schema))
                self.assertFalse('mynewtable' in tables)
                self.assertTrue('myverynewtable' in tables)
                # Rename it back
                conn.renameVectorTable(schema, 'myVeryNewTable', 'myNewTable')
                tables = self._table_names(conn.tables(schema))
                self.assertTrue('mynewtable' in tables)
                self.assertFalse('myverynewtable' in tables)

            # Vacuum
            if capabilities & QgsAbstractDatabaseProviderConnection.Vacuum:
                conn.vacuum('myNewSchema', 'myNewTable')

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema:
                # Drop schema (should fail)
                with self.assertRaises(QgsProviderConnectionException) as ex:
                    conn.dropSchema('myNewSchema')

            # Check some column types operations
            table = self._table_by_name(conn.tables(schema), 'mynewtable')
            self.assertEqual(len(table.geometryColumnTypes()), 1)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, QgsWkbTypes.LineString)
            # Add a new (existing type)
            table.addGeometryColumnType(QgsWkbTypes.LineString, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(len(table.geometryColumnTypes()), 1)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, QgsWkbTypes.LineString)
            # Add a new one
            table.addGeometryColumnType(QgsWkbTypes.LineString, QgsCoordinateReferenceSystem.fromEpsgId(4326))
            self.assertEqual(len(table.geometryColumnTypes()), 2)
            ct = table.geometryColumnTypes()[0]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(ct.wkbType, QgsWkbTypes.LineString)
            ct = table.geometryColumnTypes()[1]
            self.assertEqual(ct.crs, QgsCoordinateReferenceSystem.fromEpsgId(4326))
            self.assertEqual(ct.wkbType, QgsWkbTypes.LineString)

            # Check fields
            fields = conn.fields('myNewSchema', 'myNewTable')
            for f in ['string_t', 'long_t', 'double_t', 'integer_t', 'date_t', 'datetime_t', 'time_t']:
                self.assertTrue(f in fields.names())

            if capabilities & QgsAbstractDatabaseProviderConnection.AddField:
                field = QgsField('short_lived_field', QVariant.Int, 'integer')
                conn.addField(field, 'myNewSchema', 'myNewTable')
                fields = conn.fields('myNewSchema', 'myNewTable')
                self.assertTrue('short_lived_field' in fields.names())

                if capabilities & QgsAbstractDatabaseProviderConnection.DeleteField:
                    conn.deleteField('short_lived_field', 'myNewSchema', 'myNewTable')

                    fields = conn.fields('myNewSchema', 'myNewTable')
                    self.assertFalse('short_lived_field' in fields.names())

            # Drop table
            conn.dropVectorTable(schema, 'myNewTable')
            conn.dropVectorTable(schema, 'myNewAspatialTable')
            table_names = self._table_names(conn.tables(schema))
            self.assertFalse('myNewTable' in table_names)

            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema:
                # Drop schema
                conn.dropSchema('myNewSchema')
                self.assertFalse('myNewSchema' in conn.schemas())

        conns = md.connections()
        self.assertTrue(isinstance(list(conns.values())[0], QgsAbstractDatabaseProviderConnection))

        # Remove connection
        spy_deleted = QSignalSpy(md.connectionDeleted)
        md.deleteConnection('qgis_test1')
        self.assertEqual(list(md.connections().values()), [])
        self.assertEqual(len(spy_deleted), 1)


if __name__ == '__main__':
    unittest.main()
