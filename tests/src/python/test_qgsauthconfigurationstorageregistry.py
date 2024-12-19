"""
Test for QgsAuthConfigurationStorageRegistry class.

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthConfigurationStorageRegistry -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

import os
from osgeo import gdal
import unittest
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    Qgis,
    QgsAuthConfigurationStorageRegistry,
    QgsAuthMethodConfig,
    QgsAuthConfigurationStorageDb,
)
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath


class TestQgsAuthConfigurationStorageRegistry(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        start_app()

    def setUp(self):
        """Run before each test"""

        super().setUpClass()
        self.temp_dir = QTemporaryDir()
        self.temp_dir_path = self.temp_dir.path()

        # Create an empty sqlite database using GDAL
        self.db_path = os.path.join(self.temp_dir_path, "test.sqlite")
        ds = gdal.GetDriverByName("SQLite").Create(self.db_path, 0, 0, 0)
        del ds

        # Verify that the file was created
        assert os.path.exists(self.db_path)

        self.storage = QgsAuthConfigurationStorageDb("QSQLITE:" + self.db_path)
        assert self.storage.initialize()

        assert self.storage.type() == "DB-QSQLITE"

    def testStorageRegistry(self):
        """Test storage registry"""

        registry = QgsAuthConfigurationStorageRegistry()

        spy_added = QSignalSpy(registry.storageAdded)
        spy_changed = QSignalSpy(registry.storageChanged)
        spy_removed = QSignalSpy(registry.storageRemoved)

        registry.addStorage(self.storage)

        self.assertEqual(len(spy_added), 1)
        self.assertEqual(len(spy_changed), 0)
        self.assertEqual(len(spy_removed), 0)

        # Create a new configuration
        config = QgsAuthMethodConfig()
        config.setId("test")
        config.setName("Test")
        config.setMethod("basic")
        config.setConfig("username", "test")
        config.setConfig("password", "test")
        payload = config.configString()

        self.assertTrue(self.storage.storeMethodConfig(config, payload))

        self.assertEqual(len(spy_added), 1)
        self.assertEqual(len(spy_changed), 1)
        self.assertEqual(len(spy_removed), 0)

        self.assertIn(self.storage, registry.storages())

        registry.removeStorage(self.storage.id())

        self.assertEqual(len(spy_added), 1)
        self.assertEqual(len(spy_changed), 1)
        self.assertEqual(len(spy_removed), 1)


if __name__ == "__main__":
    unittest.main()
