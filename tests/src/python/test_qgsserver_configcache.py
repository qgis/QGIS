"""QGIS Unit tests for QgsConfigCache

From build dir, run: ctest -R PyQgsServerConfigCache -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'David Marteau'
__date__ = '28/01/2022'
__copyright__ = 'Copyright 2015, The QGIS Project'

import os
import sys
import qgis  # NOQA

from pathlib import Path
from time import time

from qgis.testing import unittest
from utilities import unitTestDataPath
from qgis.server import (QgsConfigCache,
                         QgsServerSettings)
from qgis.core import QgsApplication, QgsProject

from test_qgsserver import QgsServerTestBase


class TestQgsServerConfigCache(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        """Run before all tests"""
        self._app = QgsApplication([], False)

    @classmethod
    def tearDownClass(self):
        """Run after all tests"""
        self._app.exitQgis

    def test_periodic_cache_strategy(self):

        path = Path(unitTestDataPath('qgis_server_project')) / 'project.qgs'
        self.assertTrue(path.exists())

        os.environ['QGIS_SERVER_PROJECT_CACHE_STRATEGY'] = 'periodic'
        os.environ['QGIS_SERVER_PROJECT_CACHE_CHECK_INTERVAL'] = '3000'
        settings = QgsServerSettings()
        settings.load()

        self.assertEqual(settings.projectCacheStrategy(), 'periodic')
        self.assertEqual(settings.projectCacheCheckInterval(), 3000)

        cache = QgsConfigCache(settings)

        self.assertEqual(cache.strategyName(), "periodic")

        prj1 = cache.project(str(path))
        self.assertIsNotNone(prj1)
        self.assertEqual(prj1.readEntry("TestConfigCache", "/", ""), ("", False))

        # Change entry
        prj1.writeEntry("TestConfigCache", "/", "foobar")

        # Query cache again
        prj2 = cache.project(str(path))
        self.assertIsNotNone(prj2)
        # Ensure project is not reloaded
        self.assertEqual(prj2.readEntry("TestConfigCache", "/"), ('foobar', True))

        # Change project modified time
        path.touch()

        # Some delay
        st = time()
        while time() - st < 1:
            self._app.processEvents()

        # Query cache again
        # Project should no be reloaded yet
        prj3 = cache.project(str(path))
        self.assertIsNotNone(prj3)
        # Ensure project is not reloaded
        self.assertEqual(prj3.readEntry("TestConfigCache", "/"), ('foobar', True))

        # Some delay
        st = time()
        while time() - st < 3:
            self._app.processEvents()

        # Project should be reloaded now
        prj4 = cache.project(str(path))
        self.assertIsNotNone(prj4)
        # Ensure project is reloaded
        self.assertEqual(prj4.readEntry("TestConfigCache", "/", ""), ("", False))

    def test_file_watcher_invalidation(self):

        path = Path(unitTestDataPath('qgis_server_project')) / 'project.qgs'
        self.assertTrue(path.exists())

        os.environ['QGIS_SERVER_PROJECT_CACHE_STRATEGY'] = 'filesystem'
        settings = QgsServerSettings()
        settings.load()

        self.assertEqual(settings.projectCacheStrategy(), 'filesystem')

        cache = QgsConfigCache(settings)

        self.assertEqual(cache.strategyName(), "filesystem")

        prj1 = cache.project(str(path))
        self.assertIsNotNone(prj1)
        self.assertEqual(prj1.readEntry("TestConfigCache", "/", ""), ("", False))

        # Change entry
        prj1.writeEntry("TestConfigCache", "/", "foobaz")

        # Query cache again
        prj2 = cache.project(str(path))
        self.assertIsNotNone(prj2)
        # Ensure project is not reloaded
        self.assertEqual(prj2.readEntry("TestConfigCache", "/"), ('foobaz', True))

        # Change project modified time
        path.touch()

        # Some delay
        st = time()
        while time() - st < 1:
            self._app.processEvents()

        # Query cache again
        # Project should be reloaded
        prj3 = cache.project(str(path))
        self.assertIsNotNone(prj3)
        # Ensure project is reloaded
        self.assertEqual(prj3.readEntry("TestConfigCache", "/"), ("", False))

    def test_null_invalidation(self):

        path = Path(unitTestDataPath('qgis_server_project')) / 'project.qgs'
        self.assertTrue(path.exists())

        os.environ['QGIS_SERVER_PROJECT_CACHE_STRATEGY'] = 'off'
        settings = QgsServerSettings()
        settings.load()

        self.assertEqual(settings.projectCacheStrategy(), 'off')

        cache = QgsConfigCache(settings)

        self.assertEqual(cache.strategyName(), "off")

        prj1 = cache.project(str(path))
        self.assertIsNotNone(prj1)
        self.assertEqual(prj1.readEntry("TestConfigCache", "/", ""), ("", False))

        # Change entry
        prj1.writeEntry("TestConfigCache", "/", "barbaz")

        # Query cache again
        prj2 = cache.project(str(path))
        self.assertIsNotNone(prj2)
        # Ensure project is not reloaded
        self.assertEqual(prj2.readEntry("TestConfigCache", "/"), ('barbaz', True))

        # Change project modified time
        path.touch()

        # Some delay
        st = time()
        while time() - st < 2:
            self._app.processEvents()

        # Query cache again
        # Project should no be reloaded
        prj3 = cache.project(str(path))
        self.assertIsNotNone(prj3)
        # Ensure project is reloaded
        self.assertEqual(prj3.readEntry("TestConfigCache", "/"), ("barbaz", True))

    def test_default_strategy_setting(self):

        os.environ['QGIS_SERVER_PROJECT_CACHE_STRATEGY'] = ''
        settings = QgsServerSettings()
        settings.load()

        self.assertEqual(settings.projectCacheStrategy(), 'filesystem')

    def test_initialized_instance(self):

        os.environ['QGIS_SERVER_PROJECT_CACHE_STRATEGY'] = 'off'
        settings = QgsServerSettings()
        settings.load()

        QgsConfigCache.initialize(settings)

        self.assertEqual(QgsConfigCache.instance().strategyName(), 'off')


if __name__ == "__main__":
    unittest.main()
