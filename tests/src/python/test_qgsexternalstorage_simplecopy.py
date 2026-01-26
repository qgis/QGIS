"""QGIS Unit tests for Simple copy external storage

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Julien Cabieces"
__date__ = "31/03/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.testing import unittest

from test_qgsexternalstorage_base import TestPyQgsExternalStorageBase


class TestPyQgsExternalStorageSimpleCopy(
    TestPyQgsExternalStorageBase, unittest.TestCase
):

    storageType = "SimpleCopy"
    badUrl = "/nothing/here/"

    @classmethod
    def setUpClass(cls):
        """Run before all tests:"""
        super().setUpClass()
        unittest.TestCase.setUpClass()

        cls.temp_dir = QTemporaryDir()
        cls.url = cls.temp_dir.path()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.temp_dir = None
        super().tearDownClass()
        unittest.TestCase.tearDownClass()

    def testStoreMissingAuth(self):
        """Override this one because there is authentication for SimpleCopy external storage"""
        pass


if __name__ == "__main__":
    unittest.main()
