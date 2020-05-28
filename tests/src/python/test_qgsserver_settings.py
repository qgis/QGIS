# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServerSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Paul Blottiere'
__date__ = '20/12/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import os

from qgis.PyQt.QtCore import QCoreApplication

from utilities import unitTestDataPath
from qgis.testing import unittest
from qgis.server import QgsServerSettings


class TestQgsServerSettings(unittest.TestCase):

    def setUp(self):
        self.settings = QgsServerSettings()
        self.testdata_path = unitTestDataPath("qgis_server_settings")

    def tearDown(self):
        pass

    def test_env_parallel_rendering(self):
        env = "QGIS_SERVER_PARALLEL_RENDERING"

        # test parallel rendering value from environment variable
        os.environ[env] = "1"
        self.settings.load()
        self.assertTrue(self.settings.parallelRendering())
        os.environ.pop(env)

        os.environ[env] = "0"
        self.settings.load()
        self.assertFalse(self.settings.parallelRendering())
        os.environ.pop(env)

    def test_env_log_level(self):
        env = "QGIS_SERVER_LOG_LEVEL"

        # test log level value from environment variable
        os.environ[env] = "3"
        self.settings.load()
        self.assertEqual(self.settings.logLevel(), 3)
        os.environ.pop(env)

        os.environ[env] = "1"
        self.settings.load()
        self.assertEqual(self.settings.logLevel(), 1)
        os.environ.pop(env)

    def test_env_log_file(self):
        env = "QGIS_SERVER_LOG_FILE"

        # test parallel rendering value from environment variable
        os.environ[env] = "/tmp/qgisserv.log"
        self.settings.load()
        self.assertEqual(self.settings.logFile(), "/tmp/qgisserv.log")
        os.environ.pop(env)

        os.environ[env] = "/tmp/qserv.log"
        self.settings.load()
        self.assertEqual(self.settings.logFile(), "/tmp/qserv.log")
        os.environ.pop(env)

    def test_env_project_file(self):
        env = "QGIS_PROJECT_FILE"

        # test parallel rendering value from environment variable
        os.environ[env] = "/tmp/myproject.qgs"
        self.settings.load()
        self.assertEqual(self.settings.projectFile(), "/tmp/myproject.qgs")
        os.environ.pop(env)

        os.environ[env] = "/tmp/myproject2.qgs"
        self.settings.load()
        self.assertEqual(self.settings.projectFile(), "/tmp/myproject2.qgs")
        os.environ.pop(env)

    def test_env_max_cache_layers(self):
        env = "MAX_CACHE_LAYERS"

        # test parallel rendering value from environment variable
        os.environ[env] = "3"
        self.settings.load()
        self.assertEqual(self.settings.maxCacheLayers(), 3)
        os.environ.pop(env)

        os.environ[env] = "100"
        self.settings.load()
        self.assertEqual(self.settings.maxCacheLayers(), 100)
        os.environ.pop(env)

    def test_env_max_threads(self):
        env = "QGIS_SERVER_MAX_THREADS"

        # test parallel rendering value from environment variable
        os.environ[env] = "3"
        self.settings.load()
        self.assertEqual(self.settings.maxThreads(), 3)
        os.environ.pop(env)

        os.environ[env] = "5"
        self.settings.load()
        self.assertEqual(self.settings.maxThreads(), 5)
        os.environ.pop(env)

    def test_env_cache_size(self):
        env = "QGIS_SERVER_CACHE_SIZE"

        self.assertEqual(self.settings.cacheSize(), 50 * 1024 * 1024)

        os.environ[env] = "1024"
        self.settings.load()
        self.assertEqual(self.settings.cacheSize(), 1024)
        os.environ.pop(env)

    def test_env_cache_directory(self):
        env = "QGIS_SERVER_CACHE_DIRECTORY"

        os.environ[env] = "/tmp/fake"
        self.settings.load()
        self.assertEqual(self.settings.cacheDirectory(), "/tmp/fake")
        os.environ.pop(env)

    def test_priority(self):
        env = "QGIS_OPTIONS_PATH"
        dpath = "conf0"
        QCoreApplication.setOrganizationName(dpath)

        # load settings
        os.environ[env] = self.testdata_path
        self.settings.load()

        # test conf
        self.assertTrue(self.settings.parallelRendering())
        self.assertEqual(self.settings.maxThreads(), 3)

        # set environment variables and test priority
        env_pr = "QGIS_SERVER_PARALLEL_RENDERING"
        os.environ[env_pr] = "0"

        env_mt = "QGIS_SERVER_MAX_THREADS"
        os.environ[env_mt] = "5"

        self.settings.load()
        self.assertFalse(self.settings.parallelRendering())
        self.assertEqual(self.settings.maxThreads(), 5)

        # clear environment
        os.environ.pop(env)
        os.environ.pop(env_pr)
        os.environ.pop(env_mt)

    def test_options_path_conf0(self):
        env = "QGIS_OPTIONS_PATH"
        dpath = "conf0"
        ini = "{0}.ini".format(os.path.join(self.testdata_path, dpath))
        QCoreApplication.setOrganizationName(dpath)

        # load settings
        os.environ[env] = self.testdata_path
        self.settings.load()

        # test ini file
        self.assertEqual(ini, self.settings.iniFile())

        # test conf
        self.assertTrue(self.settings.parallelRendering())
        self.assertEqual(self.settings.maxThreads(), 3)
        self.assertEqual(self.settings.cacheSize(), 52428800)

        # default value when an empty string is indicated in ini file
        self.assertEqual(self.settings.cacheDirectory(), "cache")

        # clear environment
        os.environ.pop(env)

    def test_options_path_conf1(self):
        env = "QGIS_OPTIONS_PATH"
        dpath = "conf1"
        ini = "{0}.ini".format(os.path.join(self.testdata_path, dpath))
        QCoreApplication.setOrganizationName(dpath)

        # load settings
        os.environ[env] = self.testdata_path
        self.settings.load()

        # test ini file
        self.assertEqual(ini, self.settings.iniFile())

        # test conf
        self.assertFalse(self.settings.parallelRendering())
        self.assertEqual(self.settings.maxThreads(), 5)
        self.assertEqual(self.settings.cacheSize(), 52428800)
        self.assertEqual(self.settings.cacheDirectory(), "/tmp/mycache")

        # clear environment
        os.environ.pop(env)


if __name__ == '__main__':
    unittest.main()
