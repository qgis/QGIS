# -*- coding: utf-8 -*-
"""QGIS Unit tests for Simple copy external storage

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Julien Cabieces'
__date__ = '31/03/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

from shutil import rmtree
import os
import tempfile
import time

from utilities import unitTestDataPath, waitServer
from test_qgsexternalstorage_base import TestPyQgsExternalStorageBase

from qgis.PyQt.QtCore import QCoreApplication, QEventLoop, QUrl, QTemporaryDir

from qgis.core import (
    QgsApplication,
    QgsAuthMethodConfig,
    QgsExternalStorageFetchedContent)

from qgis.testing import (
    start_app,
    unittest,
)


class TestPyQgsExternalStorageSimpleCopy(TestPyQgsExternalStorageBase, unittest.TestCase):

    storageType = "SimpleCopy"
    badUrl = "/nothing/here/"

    @classmethod
    def setUpClass(cls):
        """Run before all tests:"""

        cls.temp_dir = QTemporaryDir()
        cls.url = cls.temp_dir.path()

        super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        super().tearDownClass()
        cls.temp_dir = None

    def testStoreMissingAuth(self):
        """Override this one because there is authentication for SimpleCopy external storage"""
        pass


if __name__ == '__main__':
    unittest.main()
