# -*- coding: utf-8 -*-
"""QGIS Unit tests for HANA QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Maksim Rylov'
__date__ = '02/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from hdbcli import dbapi
import os
from qgis.core import (
    QgsAbstractDatabaseProviderConnection,
    QgsDataSourceUri,
    QgsProviderRegistry)
from qgis.testing import start_app, unittest
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase
from test_hana_utils import QgsHanaProviderUtils


class TestPyQgsProviderConnectionHana(unittest.TestCase, TestPyQgsProviderConnectionBase):
    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "hana" or "ogr")
    providerKey = 'hana'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        TestPyQgsProviderConnectionBase.setUpClass()

        cls.uri = 'driver=\'/usr/sap/hdbclient/libodbcHDB.so\' host=localhost port=30015 ' \
                  'user=SYSTEM password=mypassword '
        if 'QGIS_HANA_TEST_DB' in os.environ:
            cls.uri = os.environ['QGIS_HANA_TEST_DB']
        ds_uri = QgsDataSourceUri(cls.uri)
        cls.conn = dbapi.connect(address=ds_uri.host(), port=ds_uri.port(), user=ds_uri.username(),
                                 password=ds_uri.password())

        QgsHanaProviderUtils.createAndFillDefaultTables(cls.conn)
        # Create test layers
        cls.vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri + ' key=\'pk\' srid=4326 type=POINT table="qgis_test"."some_data" (geom) sql=', 'test')
        assert cls.vl.isValid()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsHanaProviderUtils.cleanUp(cls.conn)

    def createProviderMetadata(self):
        return QgsProviderRegistry.instance().providerMetadata(self.providerKey)

    def createVectorLayer(self, conn_parameters, layer_name):
        return QgsHanaProviderUtils.createVectorLayer(self.uri + ' ' + conn_parameters, layer_name)

    def testConnectionsFromUri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = self.createProviderMetadata()
        vl = self.createVectorLayer('key=\'key1\' srid=4326 type=POINT table="qgis_test"."some_data" (geom) sql=',
                                    'test')
        conn = md.createConnection(vl.dataProvider().uri().uri(), {})
        self.assertEqual(conn.uri(), QgsDataSourceUri(self.uri).uri(False))

    def testTableUri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        vl = QgsHanaProviderUtils.createVectorLayer(conn.tableUri('qgis_test', 'some_data'), 'test')
        self.assertTrue(vl.isValid())

    def testConnections(self):
        """Create some connections and retrieve them"""

        md = self.createProviderMetadata()

        conn = md.createConnection(self.uri, {})

        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateSchema))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropSchema))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.CreateVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.DropVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.RenameVectorTable))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Tables))
        self.assertTrue(bool(capabilities & QgsAbstractDatabaseProviderConnection.Schemas))

        table_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.Vector))
        self.assertEqual(table_names.sort(), ['some_data', 'some_poly_data'].sort())

        view_names = self._table_names(conn.tables('qgis_test', QgsAbstractDatabaseProviderConnection.View))
        self.assertEqual(view_names, ['some_data_view'])

    def testTrueFalse(self):
        """Test returned values from BOOL queries"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.executeSql('SELECT FALSE FROM DUMMY'), [[False]])
        self.assertEqual(conn.executeSql('SELECT TRUE FROM DUMMY'), [[True]])

    def testPrimaryKeys(self):
        """Test that PKs"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.table('qgis_test', 'some_data').primaryKeyColumns(), ['pk'])


if __name__ == '__main__':
    unittest.main()
