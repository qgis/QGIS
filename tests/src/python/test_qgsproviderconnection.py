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
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.testing import start_app, unittest
from qgis.core import (
    QgsDataSourceUri,
    QgsSettings,
    QgsProviderRegistry,
    QgsWkbTypes,
    QgsVectorLayer,
    QgsFields,
    QgsCoordinateReferenceSystem,
    QgsField,
    QgsAbstractDatabaseProviderConnection,
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
        cls.dbconn = 'dbname=\'qgis_test\''
        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(cls.dbconn + ' sslmode=disable key=\'"key1","key2"\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'postgres')
        assert cls.vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        QgsSettings().clear()

    def test_connections(self):
        """Create some connections and retrieve them"""

        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        uri = self.dbconn + ' port=5432 sslmode=disable key=\'gid\''
        dsuri = QgsDataSourceUri(uri)
        err = ''
        conn = md.connection('qgis_test1', dsuri, err)
        conn.store()

        # Retrieve capabilities
        conn = list(md.dbConnections(err).values())[0]
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertFalse(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateRasterTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameTable))

        # Retrieve schemas
        schemas = conn.schemas(err)
        self.assertEqual(err, '')
        self.assertTrue('public' in schemas)
        self.assertTrue('CamelCaseSchema' in schemas)
        self.assertTrue('qgis_test' in schemas)

        # Retrieve tables
        tables = conn.tables('qgis_test', err)
        self.assertEqual(err, '')
        self.assertTrue('someData' in tables)

        # Vacuum
        self.assertTrue(conn.vacuum('qgis_test', 'someData', err))
        self.assertEqual(err, '')

        # Rename table
        self.assertTrue(conn.renameTable('qgis_test', 'someData', 'someOtherNewData', err))
        self.assertEqual(err, '')
        tables = conn.tables('qgis_test', err)
        self.assertEqual(err, '')
        self.assertFalse('someData' in tables)
        self.assertTrue('someOtherNewData' in tables)
        self.assertTrue(conn.renameTable('qgis_test', 'someOtherNewData', 'someData', err))
        self.assertEqual(err, '')
        tables = conn.tables('qgis_test', err)
        self.assertEqual(err, '')
        self.assertTrue('someData' in tables)
        self.assertFalse('someOtherNewData' in tables)

        # Rename schema
        self.assertTrue(conn.renameSchema('qgis_test', 'qgis_test_new', err))
        self.assertEqual(err, '')
        schemas = conn.schemas(err)
        self.assertEqual(err, '')
        self.assertFalse('qgis_test' in schemas)
        self.assertTrue('qgis_test_new' in schemas)
        self.assertTrue(conn.renameSchema('qgis_test_new', 'qgis_test', err))
        self.assertEqual(err, '')
        schemas = conn.schemas(err)
        self.assertEqual(err, '')
        self.assertTrue('qgis_test' in schemas)
        self.assertFalse('qgis_test_new' in schemas)

        # Create table
        if 'myNewTable' in conn.tables('qgis_test', err):
            conn.dropTable('qgis_test', 'myNewTable', err)
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
        self.assertTrue(conn.createVectorTable('qgis_test', 'myNewTable', fields, typ, crs, True, options, err))
        self.assertEqual(err, '')
        tables = conn.tables('qgis_test', err)
        self.assertEqual(err, '')
        self.assertTrue('myNewTable' in tables)

        # Drop table
        self.assertTrue(conn.dropTable('qgis_test', 'myNewTable', err))
        self.assertEqual(err, '')
        tables = conn.tables('qgis_test', err)
        self.assertEqual(err, '')
        self.assertFalse('myNewTable' in tables)

        # Remove connection
        conn.remove()
        self.assertEqual(list(md.dbConnections(err).values()), [])


if __name__ == '__main__':
    unittest.main()
