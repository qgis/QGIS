# -*- coding: utf-8 -*-
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
)
from qgis.testing import unittest
from osgeo import gdal
from qgis.PyQt.QtCore import QTemporaryDir


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

    def test_confguration(self):
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

    def test_gpkg_fields(self):
        """Test fields"""

        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.uri, {})
        fields = conn.fields('qgis_test', 'someData')
        self.assertEqual(fields.names(), ['pk', 'cnt', 'name', 'name2', 'num_char'])


if __name__ == '__main__':
    unittest.main()
