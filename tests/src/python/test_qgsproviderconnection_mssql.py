"""QGIS Unit tests for MSSQL QgsAbastractProviderConnection API.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '12/03/2020'
__copyright__ = 'Copyright 2019, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from qgis.core import (
    QgsVectorLayer,
    QgsProviderRegistry,
    QgsDataSourceUri,
)
from qgis.testing import unittest

from test_qgsproviderconnection_base import TestPyQgsProviderConnectionBase


class TestPyQgsProviderConnectionMssql(unittest.TestCase, TestPyQgsProviderConnectionBase):

    # Provider test cases must define the string URI for the test
    uri = ''
    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'mssql'

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        TestPyQgsProviderConnectionBase.setUpClass()

        # These are the connection details for the SQL Server instance running on Travis
        cls.dbconn = "service='testsqlserver' user=sa password='<YourStrong!Passw0rd>' "
        if 'QGIS_MSSQLTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_MSSQLTEST_DB']

        cls.uri = cls.dbconn

        try:
            md = QgsProviderRegistry.instance().providerMetadata('mssql')
            conn = md.createConnection(cls.uri, {})
            conn.executeSql('drop schema [myNewSchema]')
        except:
            pass

    def test_configuration(self):
        """Test storage and retrieval for configuration parameters"""

        uri = 'dbname=\'qgis_test\' service=\'driver={SQL Server};server=localhost;port=1433;database=qgis_test\' user=\'sa\' password=\'<YourStrong!Passw0rd>\' srid=4326 type=Point estimatedMetadata=\'true\' disableInvalidGeometryHandling=\'1\' table="qgis_test"."someData" (geom)'
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(uri, {})
        ds_uri = QgsDataSourceUri(conn.uri())
        self.assertEqual(ds_uri.username(), 'sa')
        self.assertEqual(ds_uri.database(), 'qgis_test')
        self.assertEqual(ds_uri.table(), '')
        self.assertEqual(ds_uri.schema(), '')
        self.assertEqual(ds_uri.geometryColumn(), '')
        self.assertTrue(ds_uri.useEstimatedMetadata())
        self.assertEqual(ds_uri.srid(), '')
        self.assertEqual(ds_uri.password(), '<YourStrong!Passw0rd>')
        self.assertEqual(ds_uri.param('disableInvalidGeometryHandling'), '1')

        conn.store('coronavirus')
        conn = md.findConnection('coronavirus', False)
        ds_uri = QgsDataSourceUri(conn.uri())
        self.assertEqual(ds_uri.username(), 'sa')
        self.assertEqual(ds_uri.database(), 'qgis_test')
        self.assertEqual(ds_uri.table(), '')
        self.assertEqual(ds_uri.schema(), '')
        self.assertTrue(ds_uri.useEstimatedMetadata())
        self.assertEqual(ds_uri.geometryColumn(), '')
        self.assertEqual(ds_uri.srid(), '')
        self.assertEqual(ds_uri.password(), '<YourStrong!Passw0rd>')
        self.assertEqual(ds_uri.param('disableInvalidGeometryHandling'), 'true')
        conn.remove('coronavirus')

    def test_mssql_connections_from_uri(self):
        """Create a connection from a layer uri and retrieve it"""

        md = QgsProviderRegistry.instance().providerMetadata('mssql')

    def test_table_uri(self):
        """Create a connection from a layer uri and create a table URI"""

        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.uri, {})
        vl = QgsVectorLayer(conn.tableUri('qgis_test', 'someData'), 'my', 'mssql')
        self.assertTrue(vl.isValid())

    def test_mssql_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('qgis_test', 'someData')
        self.assertEqual(fields.names(), ['pk', 'cnt', 'name', 'name2', 'num_char', 'dt', 'date', 'time'])

    def test_schemas_filtering(self):
        """Test schemas filtering"""

        md = QgsProviderRegistry.instance().providerMetadata('mssql')

        conn = md.createConnection(self.uri, {})
        schemas = conn.schemas()
        self.assertEqual(len(schemas), 2)
        self.assertEqual(schemas, ['dbo', 'qgis_test'])
        filterUri = QgsDataSourceUri(self.uri)
        filterUri.setParam('excludedSchemas', 'dbo')
        conn = md.createConnection(filterUri.uri(), {})
        schemas = conn.schemas()
        self.assertEqual(len(schemas), 1)
        self.assertEqual(schemas, ['qgis_test'])

        # Store the connection
        conn.store('filteredConnection')

        otherConn = md.createConnection('filteredConnection')
        schemas = otherConn.schemas()
        self.assertEqual(len(schemas), 1)
        self.assertEqual(schemas, ['qgis_test'])

    def test_exec_sql(self):

        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.uri, {})

        results = conn.executeSql('select * from qgis_test.some_poly_data')

        rows = []
        results2 = conn.execSql('select * from qgis_test.some_poly_data')

        while results2.hasNextRow():
            rows.append(results2.nextRow())

        self.assertEqual(len(rows), 4)
        self.assertEqual(rows, results)


if __name__ == '__main__':
    unittest.main()
