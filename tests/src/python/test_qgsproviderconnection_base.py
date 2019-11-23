# -*- coding: utf-8 -*-
"""QGIS Base Unit tests for QgsAbastractProviderConnection API.

Providers must implement a test based on TestPyQgsProviderConnectionBase

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
from qgis.PyQt import QtCore


class TestPyQgsProviderConnectionBase():

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = ''

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        QgsSettings().clear()

    def _test_save_load(self, md, uri):
        """Common tests on connection save and load"""
        conn = md.createConnection(self.uri, {})
        md.saveConnection(conn, 'qgis_test1')
        # Check that we retrieve the new connection
        self.assertTrue('qgis_test1' in md.connections().keys())
        self.assertTrue('qgis_test1' in md.dbConnections().keys())
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
                capabilities & QgsAbstractDatabaseProviderConnection.DropSchema):
            if capabilities & QgsAbstractDatabaseProviderConnection.DropSchema and 'myNewSchema' in conn.schemas():
                conn.dropSchema('myNewSchema', True)
            # Create
            conn.createSchema('myNewSchema')
            schemas = conn.schemas()
            self.assertTrue('myNewSchema' in schemas)
            # Create again
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.createSchema('myNewSchema')
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
                capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable and
                capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable):

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
            self.assertEqual(table_property.maxCoordinateDimensions(), 2)
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.tableName(), 'myNewTable')
            self.assertEqual(table_property.geometryColumnCount(), 1)
            self.assertEqual(table_property.geometryColumnTypes()[0].wkbType, QgsWkbTypes.LineString)
            self.assertEqual(table_property.geometryColumnTypes()[0].crs, QgsCoordinateReferenceSystem.fromEpsgId(3857))
            self.assertEqual(table_property.defaultName(), 'myNewTable')

            # Check aspatial tables
            conn.createVectorTable(schema, 'myNewAspatialTable', fields, QgsWkbTypes.NoGeometry, crs, True, options)
            table_properties = conn.tables(schema, QgsAbstractDatabaseProviderConnection.Aspatial)
            table_property = self._table_by_name(table_properties, 'myNewAspatialTable')
            self.assertIsNotNone(table_property)
            self.assertEqual(table_property.maxCoordinateDimensions(), 0)
            self.assertEqual(table_property.tableName(), 'myNewAspatialTable')
            self.assertEqual(table_property.geometryColumnCount(), 0)
            self.assertEqual(table_property.geometryColumn(), '')
            self.assertEqual(table_property.defaultName(), 'myNewAspatialTable')
            self.assertEqual(table_property.geometryColumnTypes()[0].wkbType, QgsWkbTypes.NoGeometry)
            self.assertFalse(table_property.geometryColumnTypes()[0].crs.isValid())
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
                sql = "INSERT INTO %s (string, long, double, integer, date, datetime, time) VALUES ('QGIS Rocks - \U0001f604', 666, 1.234, 1234, '2019-07-08', '2019-07-08T12:00:12', '12:00:13.00')" % table
                res = conn.executeSql(sql)
                self.assertEqual(res, [])
                sql = "SELECT string, long, double, integer, date, datetime FROM %s" % table
                res = conn.executeSql(sql)
                # GPKG has no type for time and spatialite has no support for dates and time ...
                if self.providerKey == 'spatialite':
                    self.assertEqual(res, [['QGIS Rocks - \U0001f604', 666, 1.234, 1234, '2019-07-08', '2019-07-08T12:00:12']])
                else:
                    self.assertEqual(res, [['QGIS Rocks - \U0001f604', 666, 1.234, 1234, QtCore.QDate(2019, 7, 8), QtCore.QDateTime(2019, 7, 8, 12, 0, 12)]])
                sql = "SELECT time FROM %s" % table
                res = conn.executeSql(sql)
                self.assertIn(res, ([[QtCore.QTime(12, 0, 13)]], [['12:00:13.00']]))
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

            # Query for rasters (in qgis_test schema or no schema for GPKG, spatialite has no support)
            if self.providerKey != 'spatialite':
                table_properties = conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Raster)
                # At least one raster should be there (except for spatialite)
                self.assertTrue(len(table_properties) >= 1)
                table_property = table_properties[0]
                self.assertTrue(table_property.flags() & QgsAbstractDatabaseProviderConnection.Raster)
                self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Vector)
                self.assertFalse(table_property.flags() & QgsAbstractDatabaseProviderConnection.Aspatial)

            # Rename
            conn.renameVectorTable(schema, 'myNewTable', 'myVeryNewTable')
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

            # Check some column types operations
            table = self._table_by_name(conn.tables(schema), 'myVeryNewTable')
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
        md.deleteConnection('qgis_test1')
        self.assertEqual(list(md.connections().values()), [])

    def test_errors(self):
        """Test SQL errors"""

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        conn = self._test_save_load(md, self.uri)

        if conn.capabilities() & QgsAbstractDatabaseProviderConnection.Schemas:
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.createVectorTable('notExists', 'notReally', QgsFields(), QgsWkbTypes.Point, QgsCoordinateReferenceSystem(), False, {})

        if conn.capabilities() & QgsAbstractDatabaseProviderConnection.DropVectorTable:
            with self.assertRaises(QgsProviderConnectionException) as ex:
                conn.executeSql('DROP TABLE "notExists"')

        # Remove connection
        md.deleteConnection('qgis_test1')
        self.assertEqual(list(md.connections().values()), [])

    def test_connections(self):
        """Main test"""
        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)

        # Run common tests
        conn = self._test_save_load(md, self.uri)
        self._test_operations(md, conn)
