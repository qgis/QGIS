"""
Tests for auth manager SQLITE storage API

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthManagerStorageSqlite -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
from osgeo import gdal
import unittest

from test_authmanager_storage_base import (
    AuthManagerStorageBaseTestCase,
    TestAuthManagerStorageBase,
)
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import QgsAuthConfigurationStorageDb

__author__ = "Alessandro Pasotti"
__date__ = "2024-06-24"
__copyright__ = "Copyright 2024, The QGIS Project"


class TestAuthStorageSqlite(AuthManagerStorageBaseTestCase, TestAuthManagerStorageBase):

    @classmethod
    def setUpClass(cls):
        """Run before each tests"""

        cls.temp_dir = QTemporaryDir()
        cls.temp_dir_path = cls.temp_dir.path()

        # Create an empty sqlite database using GDAL
        cls.db_path = os.path.join(cls.temp_dir_path, "test.sqlite")
        ds = gdal.GetDriverByName("SQLite").Create(cls.db_path, 0, 0, 0)
        del ds

        # Verify that the file was created
        assert os.path.exists(cls.db_path)

        uri = f"QSQLITE://{cls.db_path}"
        cls.storage_uri = uri
        cls.storage = QgsAuthConfigurationStorageDb(uri)

        assert cls.storage.type() == "DB-QSQLITE"

        super().setUpClass()

    def testCannotCreateDb(self):
        """Generic DB storage cannot create databases"""

        path = os.path.join(self.temp_dir_path, "test_not_exist.sqlite")
        storage = QgsAuthConfigurationStorageDb(self.storage_uri)
        assert not os.path.exists(path)


if __name__ == "__main__":
    unittest.main()
