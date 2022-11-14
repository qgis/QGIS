# coding=utf-8
""""Style storage tests for GPKG

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2022-11-07'
__copyright__ = 'Copyright 2022, ItOpen'

import os
from stylestoragebase import StyleStorageTestBase
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import (
    QgsProviderRegistry,
)
from qgis.testing import unittest


class StyleStorageTest(unittest.TestCase, StyleStorageTestBase):

    # Provider test cases must define the provider name (e.g. "postgres" or "ogr")
    providerKey = 'ogr'

    def setUp(self):

        super().setUp()
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()
        md = QgsProviderRegistry.instance().providerMetadata(self.providerKey)
        self.test_uri = os.path.join(self.temp_path, 'test.gpkg')
        self.assertTrue(md.createDatabase(self.test_uri)[0])
        self.uri = self.test_uri


if __name__ == '__main__':
    unittest.main()
