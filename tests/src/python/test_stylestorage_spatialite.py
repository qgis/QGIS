# coding=utf-8
""""Style storage tests for Spatialite

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-11-07'
__copyright__ = 'Copyright 2022, ItOpen'

import os
import shutil
from stylestoragebase import StyleStorageTestBase, StyleStorageTestCaseBase
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import (
    QgsProviderRegistry,
    QgsDataSourceUri,
)
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class StyleStorageTest(StyleStorageTestCaseBase, StyleStorageTestBase):

    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'spatialite'

    def setUp(self):

        super().setUp()
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()
        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        shutil.copy(os.path.join(TEST_DATA_DIR, 'provider', 'spatialite.db'), os.path.join(self.temp_path, 'spatialite.db'))
        self.test_uri = "dbname='{}'".format(os.path.join(self.temp_path, 'spatialite.db'))
        self.uri = self.test_uri

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
