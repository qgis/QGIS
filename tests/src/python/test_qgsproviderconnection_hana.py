"""QGIS Unit tests for HANA QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Maxim Rylov"
__date__ = "02/04/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.core import (
    QgsAbstractDatabaseProviderConnection,
    QgsProviderRegistry,
)
from qgis.testing import unittest

from test_hana_utils import QgsHanaProviderUtils
from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase


class TestPyQgsProviderConnectionHana(
    unittest.TestCase, TestPyQgsProviderConnectionBase
):
    # Provider test cases must define the provider name (e.g. "hana" or "ogr")
    providerKey = "hana"
    # Provider test cases must define the string URI for the test
    uri = ""
    # HANA connection object
    conn = None
    # Name of the schema
    schemaName = ""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        TestPyQgsProviderConnectionBase.setUpClass()

        cls.uri = (
            "driver='/usr/sap/hdbclient/libodbcHDB.so' host=localhost port=30015 "
            "user=SYSTEM password=mypassword sslEnabled=true sslValidateCertificate=False"
        )
        if "QGIS_HANA_TEST_DB" in os.environ:
            cls.uri = os.environ["QGIS_HANA_TEST_DB"]
        cls.conn = QgsHanaProviderUtils.createConnection(cls.uri)

        schemaPrefix = "qgis_test_providerconn"
        QgsHanaProviderUtils.dropOldTestSchemas(cls.conn, schemaPrefix)

        cls.schemaName = QgsHanaProviderUtils.generateSchemaName(cls.conn, schemaPrefix)

        QgsHanaProviderUtils.createAndFillDefaultTables(cls.conn, cls.schemaName)
        # Create test layers
        cls.vl = QgsHanaProviderUtils.createVectorLayer(
            cls.uri
            + f' key=\'pk\' srid=4326 type=POINT table="{cls.schemaName}"."some_data" (geom) sql=',
            "test",
        )

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

        QgsHanaProviderUtils.dropSchemaIfExists(cls.conn, cls.schemaName)
        cls.conn.close()
        super().tearDownClass()

    def getUniqueSchemaName(self, name):
        return "qgis_test_providerconn_" + QgsHanaProviderUtils.generateSchemaName(self.conn, name)

    def createProviderMetadata(self):
        return QgsProviderRegistry.instance().providerMetadata(self.providerKey)

    def createVectorLayer(self, conn_parameters, layer_name):
        return QgsHanaProviderUtils.createVectorLayer(
            self.uri + " " + conn_parameters, layer_name
        )

    def testConnectionsFromUri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = self.createProviderMetadata()
        vl = self.createVectorLayer(
            f'key=\'key1\' srid=4326 type=POINT table="{self.schemaName}"."some_data" ('
            "geom) sql=",
            "test",
        )
        uri = vl.dataProvider().uri().uri()
        conn = md.createConnection(uri, {})
        self.assertEqual(conn.uri(), uri)

    def testTableUri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        vl = QgsHanaProviderUtils.createVectorLayer(
            conn.tableUri(self.schemaName, "some_data"), "test"
        )

    def testConnections(self):
        """Create some connections and retrieve them"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        # Retrieve capabilities
        capabilities = conn.capabilities()
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.CreateSchema
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DropSchema
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.CreateVectorTable
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.DropVectorTable
            )
        )
        self.assertTrue(
            bool(
                capabilities
                & QgsAbstractDatabaseProviderConnection.Capability.RenameVectorTable
            )
        )
        self.assertTrue(
            bool(capabilities & QgsAbstractDatabaseProviderConnection.Capability.Tables)
        )
        self.assertTrue(
            bool(
                capabilities & QgsAbstractDatabaseProviderConnection.Capability.Schemas
            )
        )

        table_names = self._table_names(
            conn.tables(
                self.schemaName, QgsAbstractDatabaseProviderConnection.TableFlag.Vector
            )
        )
        self.assertEqual(table_names.sort(), ["some_data", "some_poly_data"].sort())

        view_names = self._table_names(
            conn.tables(
                self.schemaName, QgsAbstractDatabaseProviderConnection.TableFlag.View
            )
        )
        self.assertEqual(view_names, ["some_data_view"])

    def testTrueFalse(self):
        """Test returned values from BOOL queries"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        self.assertEqual(conn.executeSql("SELECT FALSE FROM DUMMY"), [[False]])
        self.assertEqual(conn.executeSql("SELECT TRUE FROM DUMMY"), [[True]])

    def testPrimaryKeys(self):
        """Test returned primary keys"""

        md = self.createProviderMetadata()
        conn = md.createConnection(self.uri, {})
        self.assertEqual(
            conn.table(self.schemaName, "some_data").primaryKeyColumns(), ["pk"]
        )


if __name__ == "__main__":
    unittest.main()
