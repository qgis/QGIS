# coding=utf-8
""""Style storage tests for Oracle

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-11-07'
__copyright__ = 'Copyright 2022, ItOpen'

import os
from stylestoragebase import StyleStorageTestBase, StyleStorageTestCaseBase
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import (
    QgsDataSourceUri,
    QgsProviderRegistry,
)
from qgis.testing import unittest


class StyleStorageTest(StyleStorageTestCaseBase, StyleStorageTestBase):

    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'oracle'

    def setUp(self):

        super().setUp()
        dbconn = "host=localhost dbname=XEPDB1 port=1521 user='QGIS' password='qgis'"
        if 'QGIS_ORACLETEST_DB' in os.environ:
            dbconn = os.environ['QGIS_ORACLETEST_DB']

        self.uri = dbconn

        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        md.createConnection(self.uri, {})
        conn = md.createConnection(self.uri, {})
        conn.executeSql('DELETE FROM mdsys.sdo_geom_metadata_table WHERE sdo_table_name = \'TEST_STYLES\'')

    def schemaName(self):

        return QgsDataSourceUri(self.uri).param('username')

    def tableName(self):
        """Providers may override (Oracle?)"""

        return 'TEST_STYLES_TABLE'

    def layerUri(self, conn, schema_name, table_name):
        """Providers may override if they need more complex URI generation than
        what tableUri() offers"""

        uri = QgsDataSourceUri(conn.tableUri(schema_name, table_name))
        uri.setGeometryColumn('geom')
        uri.setParam('srid', '4326')
        uri.setParam('type', 'POINT')
        return uri.uri()


if __name__ == '__main__':
    unittest.main()
