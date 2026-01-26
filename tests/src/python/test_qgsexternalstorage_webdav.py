"""QGIS Unit tests for WebDAV external storage

External storage backend must implement a test based on TestPyQgsExternalStorageBase

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Julien Cabieces"
__date__ = "31/03/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os

from qgis.testing import unittest

from test_qgsexternalstorage_base import TestPyQgsExternalStorageBase


class TestPyQgsExternalStorageWebDav(TestPyQgsExternalStorageBase, unittest.TestCase):

    storageType = "WebDAV"
    badUrl = "http://nothinghere/"

    @classmethod
    def setUpClass(cls):
        """Run before all tests:"""

        super().setUpClass()
        unittest.TestCase.setUpClass()

        cls.url = "http://{}:{}/webdav_tests".format(
            os.environ.get("QGIS_WEBDAV_HOST", "localhost"),
            os.environ.get("QGIS_WEBDAV_PORT", "80"),
        )


if __name__ == "__main__":
    unittest.main()
